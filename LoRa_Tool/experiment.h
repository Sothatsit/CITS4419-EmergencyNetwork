// Band to use for transmission
// Use 433E6 for Asia, 866E6 for EUR, and 915E6 for NA/Aus
#define BAND 915E6

// Prefix for all transmitted messages, for disambiguation
#define COMM_ID "ZZAA"
#define NODE_ID 1

// Delay for transmission rate (ms)
#define DELAY 100

// ZZA- : Computer Science, Engineering Building, Reid Balcony

// Define each parameter set
const LoRa_Params params[] = {
  //  Bandwidth,  coding rate,  frequency,  spreading factor, transmit power

  // Base case.
  {   500E3,      5,           915E6,        7,             0},  // ZZ-A

  // Changing transmit power.
  // {   500E3,      5,           915E6,        7,             4},  // ZZ-A
  // {   500E3,      5,           915E6,        7,             8},  // ZZ-B
  // {   500E3,      5,           915E6,        7,             12},  // ZZ-C
  // {   500E3,      5,           915E6,        7,             16},  // ZZ-D
  // {   500E3,      5,           915E6,        7,             20},  // ZZ-E

  // Changing bandwidth.
  // {   500E3,      5,           915E6,        7,             0},  // ZZ-E
  // {   250E3,      5,           915E6,        7,             0},  // ZZ-F
  // {   100E3,      5,           915E6,        7,             0},  // ZZ-G
  // {   50E3,       5,           915E6,        7,             0},  // ZZ-H

  // Changing spreading factor.
  // {   500E3,      5,           915E6,        7,             0},  // ZZ-I
  // {   500E3,      5,           915E6,        8,             0},  // ZZ-J
  // {   500E3,      5,           915E6,        10,            0},  // ZZ-K
  // {   500E3,      5,           915E6,        12,            0},  // ZZ-L
};

// Number of different test parameter sets
#define NUM_PARAMS (sizeof params / sizeof (LoRa_Params))

// Number of test packets sent with each set of parameters
#define PKTS_PER_PARAM 50
