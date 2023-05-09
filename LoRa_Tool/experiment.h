// Band to use for transmission
// Use 433E6 for Asia, 866E6 for EUR, and 915E6 for NA/Aus
#define BAND 915E6

// Prefix for all transmitted messages, for disambiguation
#define COMM_ID "AAAB"
#define NODE_ID 1

// Delay for transmission rate (ms)
#define DELAY 100

// Define each parameter set
const LoRa_Params params[] = {
  //  Bandwidth,  coding rate,  frequency,  spreading factor, transmit power

  // Base case.
  {   500E3,      5,           915E6,        7,             0},  // AA

  // Changing transmit power.
  // {   500E3,      5,           915E6,        7,             4},  // AA
  // {   500E3,      5,           915E6,        7,             8},  // AB
  // {   500E3,      5,           915E6,        7,             12},  // AC
  // {   500E3,      5,           915E6,        7,             16},  // AD
  // {   500E3,      5,           915E6,        7,             20},  // AE

  // Changing bandwidth.
  // {   500E3,      5,           915E6,        7,             0},  // AA
  // {   250E3,      5,           915E6,        7,             0},  // AA
  // {   100E3,      5,           915E6,        7,             0},  // AA
  // {   50E3,       5,           915E6,        7,             0},  // AAx

  // Changing spreading factor.
  // {   500E3,      5,           915E6,        7,             0},  // BA
  // {   500E3,      5,           915E6,        8,             0},  // BB
  // {   500E3,      5,           915E6,        10,            0},  // BC
  // {   500E3,      5,           915E6,        12,            0},  // BD
};

// Number of different test parameter sets
#define NUM_PARAMS (sizeof params / sizeof (LoRa_Params))

// Number of test packets sent with each set of parameters
#define PKTS_PER_PARAM 50
