// Network homomorph with NEURON network simulator implementation.

#include "neuronSimNetworkHomomorph.hpp"

// Constructors.
NeuronSimNetworkHomomorph::NeuronSimNetworkHomomorph(Network *homomorph,
                                                     MutableParm& synapseWeightsParm,
                                                     vector<vector<pair<int, int> > > *motorConnections,
                                                     Random *randomizer, int tag)
{
   this->synapseWeightsParm = synapseWeightsParm;
   this->synapseWeightsParm.initValue(randomizer);
   this->motorConnections = motorConnections;
   this->randomizer       = randomizer;
   this->tag = tag;
   network   = homomorph->clone();
   motorErrors.resize(network->numMotors, false);
}


NeuronSimNetworkHomomorph::NeuronSimNetworkHomomorph(FILE *fp,
                                                     vector<vector<pair<int, int> > > *motorConnections,
                                                     Random *randomizer)
{
   this->randomizer = randomizer;
   network          = NULL;
   load(fp);
   this->motorConnections = motorConnections;
}


// Destructor.
NeuronSimNetworkHomomorph::~NeuronSimNetworkHomomorph()
{
}


// Optimize synapses.
void NeuronSimNetworkHomomorph::optimize(int synapseOptimizedPathLength,
                                         NeuronSim *modelSim, NeuronSim *evalSim)
{
   int   i, j, k, n, p, q;
   float e;

   // Initialize optimization.
   vector<vector<Synapse *> > synapses;
   vector<vector<float> >     permutations;
   initOptimize(synapses, permutations, synapseOptimizedPathLength);

   // Hill-climb synapse weight permutations.
   n = 0;
   e = error;
   for (i = 1, j = (int)permutations.size(); i < j; i++)
   {
      for (k = 0; k < (int)synapses.size(); k++)
      {
         for (p = 0, q = (int)synapses[k].size(); p < q; p++)
         {
            synapses[k][p]->weight = permutations[i][k];
         }
      }
      evaluate(modelSim, evalSim);
      if (error < e)
      {
         n = i;
         e = error;
      }
   }
   for (k = 0; k < (int)synapses.size(); k++)
   {
      for (p = 0, q = (int)synapses[k].size(); p < q; p++)
      {
         synapses[k][p]->weight = permutations[n][k];
      }
   }
   error = e;
}


// NEURON simulation fitness evaluation.
void NeuronSimNetworkHomomorph::evaluate(NeuronSim *modelSim, NeuronSim *evalSim)
{
   evalSim->importSynapseWeights(network);
   evalSim->run();
   error = evalSim->activationDelta(modelSim);
}


// Clone.
NeuronSimNetworkHomomorph *NeuronSimNetworkHomomorph::clone()
{
   int i, n;
   NeuronSimNetworkHomomorph *simNetworkMorph;

   simNetworkMorph = new NeuronSimNetworkHomomorph(
      network, synapseWeightsParm,
      motorConnections, randomizer, tag);
   assert(simNetworkMorph != NULL);
   simNetworkMorph->error = error;
   for (i = 0, n = (int)motorErrors.size(); i < n; i++)
   {
      simNetworkMorph->motorErrors[i] = motorErrors[i];
   }
   return(simNetworkMorph);
}


// Load.
void NeuronSimNetworkHomomorph::load(FILE *fp)
{
   int  i, n;
   bool b;

   synapseWeightsParm.load(fp);
   if (network != NULL)
   {
      delete network;
   }
   network = new Network(fp);
   assert(network != NULL);
   FREAD_INT(&tag, fp);
   FREAD_FLOAT(&error, fp);
   n = (int)network->numMotors;
   motorErrors.resize(n, false);
   for (i = 0; i < n; i++)
   {
      FREAD_BOOL(&b, fp);
      motorErrors[i] = b;
   }
   FREAD_INT(&offspringCount, fp);
}


// Save.
void NeuronSimNetworkHomomorph::save(FILE *fp)
{
   int  i, n;
   bool b;

   synapseWeightsParm.save(fp);
   network->save(fp);
   FWRITE_INT(&tag, fp);
   FWRITE_FLOAT(&error, fp);
   n = (int)motorErrors.size();
   for (i = 0; i < n; i++)
   {
      b = motorErrors[i];
      FWRITE_BOOL(&b, fp);
   }
   FWRITE_INT(&offspringCount, fp);
}


// Print.
void NeuronSimNetworkHomomorph::print(bool printNetwork)
{
   int i, n;

   if (printNetwork)
   {
      printf("Network:\n");
      if (network != NULL)
      {
         network->print();
      }
      else
      {
         printf("NULL\n");
      }
   }
   printf("tag=%d\n", tag);
   printf("error=%f\n", error);
   printf("motorErrors: ");
   for (i = 0, n = (int)motorErrors.size(); i < n; i++)
   {
      if (motorErrors[i])
      {
         printf("%d ", i);
      }
   }
   printf("\n");
   printf("offspringCount=%d\n", offspringCount);
   printf("synapseWeightsParm:\n");
   synapseWeightsParm.print();
}
