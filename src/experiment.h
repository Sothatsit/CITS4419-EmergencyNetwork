// Band to use for transmission
// Use 433E6 for Asia, 866E6 for EUR, and 915E6 for NA/Aus
#define BAND 915E6

// Prefix for all transmitted messages, for disambiguation
#define COMM_ID "ZZBN"
#define NODE_ID 2

// Delay for transmission rate (ms)
#define DELAY 1000
// Used delay of 250 for ZZAA & ZZAB
// Used delay of 500 for ZZAC - ZZAG
// Used delay of 1000 for ZZAI & ZZAJ
// Used delay of 500 for ZZAK & ZZAM
// Used delay of 1000 for ZZAN
// Used delay of 1000 for all ZZB


// ZZA- : Computer Science, Engineering Building, Reid Balcony

// Define each parameter set
const LoRa_Params params[] = {
  //  Bandwidth,  coding rate,  frequency,  spreading factor, transmit power

  // Base case.
  // {   250E3,      5,           915E6,        8,             0},  // ZZ-A

  // Changing transmit power.
  // {   250E3,      5,           915E6,        8,             4},  // ZZ-B
  // {   250E3,      5,           915E6,        8,             8},  // ZZ-C
  // {   250E3,      5,           915E6,        8,             12},  // ZZ-D
  // {   250E3,      5,           915E6,        8,             16},  // ZZ-E
  // {   250E3,      5,           915E6,        8,             20},  // ZZ-F

  // Changing bandwidth.
  // {   500E3,      5,           915E6,        8,             4},  // ZZ-G
  // {   250E3,      5,           915E6,        8,             4},  // ZZ-H (Skipped as same as ZZ-B)
  // {   100E3,      5,           915E6,        8,             4},  // ZZ-I
  // {   50E3,       5,           915E6,        8,             4},  // ZZ-J

  // Changing spreading factor.
  // {   250E3,      5,           915E6,        7,             4},  // ZZ-K
  // {   250E3,      5,           915E6,        8,             4},  // ZZ-L (Skipped as same as ZZ-B)
  // {   250E3,      5,           915E6,        9,             4},  // ZZ-M
  {   250E3,      5,           915E6,        10,            4},  // ZZ-N
};

// Number of different test parameter sets
#define NUM_PARAMS (sizeof params / sizeof (LoRa_Params))

// Number of test packets sent with each set of parameters
#define PKTS_PER_PARAM 50
