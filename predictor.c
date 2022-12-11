//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"
#include <math.h>
//
// TODO:Student Information
//
const char *studentName = "SAIANUDEEP REDDY NAYINI";
const char *studentID   = "A59019035";
const char *email       = "snayini@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;
int i;
uint32_t gshare_history;
uint32_t pc_segments;
uint32_t *chooser_array;
uint32_t *lshare_bht;
uint32_t *lshare_history_table;
uint32_t *choice_predictor;
int mask_pc;
int mask;
#define THETA 31
#define CUSTOM_HISTORY  27
#define PER_LEN  333
int custom_history[CUSTOM_HISTORY];
int perceptron_table[PER_LEN][CUSTOM_HISTORY+1];
//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//Add your own Branch Predictor data structures here
//uint32_t gshare_bht_count = 1 << ghistoryBits;
uint32_t *gshare_bht;
//
//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor gshare
//
void
init_predictor_gshare()
{
uint32_t gshare_bht_count = 1 << ghistoryBits;
  gshare_bht = (uint32_t*)malloc(gshare_bht_count * sizeof(uint32_t));
  
  for (i=0; i< gshare_bht_count ; i++)
    	  gshare_bht[i] = WN;
  
  
  gshare_history = 0;
}
void
init_predictor_tournament()
{
  int gshare_bht_count = 1 << ghistoryBits;
  gshare_bht = (uint32_t*)malloc(gshare_bht_count * sizeof(uint32_t));
  
  for (i=0; i< gshare_bht_count ; i++)
    gshare_bht[i] = WN;
  
  gshare_history = 0;
  
  int lshare_bht_count = 1 << lhistoryBits;
  lshare_bht = (uint32_t*)malloc(lshare_bht_count * sizeof(uint32_t));
  
  for (i=0; i< lshare_bht_count ; i++)
    lshare_bht[i] = WN;
  
  int lshare_history_table_length = 1 << pcIndexBits ;
  lshare_history_table = (uint32_t*)malloc(lshare_history_table_length * sizeof(uint32_t));
    
  for (i=0; i< lshare_history_table_length ; i++)
    lshare_history_table[i] = 0;
  
  //int choice_predictor_length = 1 << pcIndexBits;
  int choice_predictor_length = 1 << ghistoryBits;
  choice_predictor = (uint32_t*)malloc(choice_predictor_length * sizeof(uint32_t));
    
  for (i=0; i< choice_predictor_length ; i++)
    choice_predictor[i] = WN; //WN and SN for local, ST, WT for gshare
  
}

void
init_predictor_custom()
{
int j;
	for(i=0;i<CUSTOM_HISTORY;i++)
	custom_history[i] = 0;
  //12 weights w0,w1,..w11 each of 6bits for theta of 34 or -34
  for (i=0; i<PER_LEN; i++)
	  for(j=0; j<CUSTOM_HISTORY+1; j++)
	  perceptron_table[i][j] = 0;
  mask_pc = (int)(ceil(log2(PER_LEN-1)));
  mask = ((1<<mask_pc)-1);
}



//Depending upon btype the init, make_prediction, train_prediction are called
void init_predictor()
{
  switch (bpType) {
    case STATIC:
    case GSHARE:
      init_predictor_gshare();
      break;
    case TOURNAMENT:
      init_predictor_tournament();
      break;
    case CUSTOM:
      init_predictor_custom();
      break;
    default:
      break;
  }
}
// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken

uint8_t 
make_prediction_gshare(uint32_t pc)
{
uint32_t gshare_bht_count = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (gshare_bht_count - 1); // (gshare - 1) is used in accordance with the gshare bits count
  uint32_t ghistory_lower_bits = gshare_history & (gshare_bht_count -1);
  uint32_t address = pc_lower_bits ^ ghistory_lower_bits;
  switch(gshare_bht[address])
  {
    case 0:
      return NOTTAKEN;
      //break;
  case 1:
      return NOTTAKEN;
     // break;
  case 2:
      return TAKEN;
     // break;
  case 3:
      return TAKEN;
      //break;
  default:
      printf("Invalid Entry");
      return NOTTAKEN;
    }    
}

uint8_t 
make_prediction_tournament(uint32_t pc)
{
  uint32_t gshare_bht_count = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (gshare_bht_count - 1); // (gshare - 1) is used in accordance with the gshare bits count
  uint32_t ghistory_lower_bits = gshare_history & (gshare_bht_count -1);
  //uint32_t address = pc_lower_bits & ghistory_lower_bits ;
  uint32_t address = ghistory_lower_bits ;
  uint32_t pc_low_lshare = pc & ((1<<pcIndexBits)-1);
  //int lshare_bht_address = lshare_history_table[pc_low_lshare] & pc_low_lshare;
  int lshare_bht_address = lshare_history_table[pc_low_lshare] & ((1<<lhistoryBits)-1);
  uint32_t address_choice = ghistory_lower_bits ;

  
 if(choice_predictor[address_choice] == WN || choice_predictor[address_choice] == SN)
 {
	 switch(lshare_bht[lshare_bht_address])
	 {
		 case WN:
			 return NOTTAKEN;
		 case SN:
			 return NOTTAKEN;
		 case WT:
			 return TAKEN;
		 case ST:
			 return TAKEN;
		 default:
			 printf("Invalid");
			 return NOTTAKEN;
	 }
 }
 else 
 {	 switch(gshare_bht[address])
	 {
		 case WN:
			 return NOTTAKEN;
		 case SN:
			 return NOTTAKEN;
		 case WT:
			 return TAKEN;
		 case ST:
			 return TAKEN;
		 default:
			 printf("Invalid");
			 return NOTTAKEN;
	 }
 }
 }
uint8_t 
make_prediction_custom(uint32_t pc)
{
	int pc_lower_bits_custom = pc % PER_LEN;
	int bias = perceptron_table[pc_lower_bits_custom][0];
	int y; // dot product of history and perceptrons
	for(i=0;i<CUSTOM_HISTORY;i++)
		y = y+perceptron_table[pc_lower_bits_custom][i+1]*custom_history[i];
	y = y+bias;
	int outcome;
	outcome = y>=0 ? TAKEN : NOTTAKEN;
 return outcome; 
}

uint8_t
make_prediction(uint32_t pc)
{
uint8_t outcome_merge;
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
     outcome_merge = make_prediction_gshare(pc);
  return outcome_merge;
  break;
    case TOURNAMENT:
     outcome_merge = make_prediction_tournament(pc); 
  return outcome_merge;
  break;
    case CUSTOM:
     outcome_merge = make_prediction_custom(pc); 
  return outcome_merge;
  break;
    default:
      break;
  }
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
void
train_predictor_gshare(uint32_t pc, uint8_t outcome)
{
pc_segments = 1 << pcIndexBits;
uint32_t gshare_bht_count = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (gshare_bht_count - 1); // (gshare - 1) is used in accordance with the gshare bits count
  uint32_t ghistory_lower_bits = gshare_history & (gshare_bht_count -1);
  uint32_t address = pc_lower_bits ^ ghistory_lower_bits;
  switch(gshare_bht[address])
  {
    case WN:
      gshare_bht[address] = (outcome == TAKEN) ? WT: SN ;
      break;
    case WT:
      gshare_bht[address] = (outcome == TAKEN) ? ST: WN ;
      break;
    case ST:
      gshare_bht[address] = (outcome == TAKEN) ? ST: WT ;
      break;
    case SN:
      gshare_bht[address] = (outcome == TAKEN) ? WN: SN ;
      break;
  default:
      printf("Invalid Entry present in the Branch History Table");
  }
 if(outcome == TAKEN)
 { if ((gshare_history<<1) +1 >= gshare_bht_count-1)
		 gshare_history = ((gshare_history<<1) +1) & (gshare_bht_count-1);
	 else		 
          gshare_history = (gshare_history << 1) + 1;}
 else
 {
	 if((gshare_history <<1) >= gshare_bht_count-1)
	 gshare_history = (gshare_history<<1) & (gshare_bht_count -1);
	 else
		 gshare_history = gshare_history<<1;
 }
 }

void
train_predictor_tournament(uint32_t pc, uint8_t outcome)
{
//
  //Implement Predictor training
  uint32_t gshare_bht_count = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (gshare_bht_count - 1); // (gshare - 1) is used in accordance with the gshare bits count
  uint32_t ghistory_lower_bits = gshare_history & (gshare_bht_count -1);
 // uint32_t address_gshare_bht = pc_lower_bits ^ ghistory_lower_bits ;
  uint32_t address_choice = ghistory_lower_bits ;
 uint32_t address_gshare_bht = ghistory_lower_bits ;
  uint32_t pc_low_lshare = pc & ((1<<pcIndexBits) - 1);
  //int lshare_bht_addr = lshare_history_table[pc_low_lshare] & pc_low_lshare;
  int lshare_bht_addr = lshare_history_table[pc_low_lshare] & ((1<<lhistoryBits) - 1);
  
int lshare =  lshare_bht[lshare_bht_addr];
if(lshare == WN || lshare== SN)
{lshare = NOTTAKEN;}
else
{lshare = TAKEN;}

  switch(lshare_bht[lshare_bht_addr]){
      case WN:
      lshare_bht[lshare_bht_addr] = (outcome == TAKEN) ? WT: SN ;
      break;
    case WT:
      lshare_bht[lshare_bht_addr] = (outcome == TAKEN) ? ST: WN ;
      break;
    case ST:
      lshare_bht[lshare_bht_addr] = (outcome == TAKEN) ? ST: WT ;
      break;
    case SN:
      lshare_bht[lshare_bht_addr] = (outcome == TAKEN) ? WN: SN ;
      break;
    default:
      printf("Invalid Entry present in the Branch History Table");
  }

lshare_history_table[pc_low_lshare] = ((lshare_history_table[pc_low_lshare] << 1) | outcome);

int gshare =  gshare_bht[address_gshare_bht];
if(gshare == WN || gshare == SN)
{gshare = NOTTAKEN;}
else
{gshare = TAKEN;}




  switch(gshare_bht[address_gshare_bht]){
    case WN:
      gshare_bht[address_gshare_bht] = (outcome == TAKEN) ? WT: SN ;
      break;
    case WT:
      gshare_bht[address_gshare_bht] = (outcome == TAKEN) ? ST: WN ;
      break;
    case ST:
      gshare_bht[address_gshare_bht] = (outcome == TAKEN) ? ST: WT ;
      break;
    case SN:
      gshare_bht[address_gshare_bht] = (outcome == TAKEN) ? WN: SN ;
      break;
  default:
      printf("Invalid Entry present in the Branch History Table");
  }

gshare_history = ((gshare_history << 1) | outcome);
  
/*switch(choice_predictor[address_choice]){
	case SN:
		choice_predictor[address_choice] = (outcome == gshare && outcome!=lshare) ? WN : SN;
		break;
	case WN:
		choice_predictor[address_choice] = (outcome == gshare && outcome!=lshare) ? WT : SN;
		break;
	case WT:
		choice_predictor[address_choice] = (outcome==lshare) ? WN : ST;
		break;
	case ST:
		choice_predictor[address_choice] = (outcome==lshare) ? WT : ST;
		break;
	default:
		printf("Invalid entry");
}*/
if(gshare == outcome & lshare!=outcome & choice_predictor[address_choice]!=3)
	choice_predictor[address_choice]++; 
else if(gshare != outcome & lshare==outcome & choice_predictor[address_choice]!=0)
	choice_predictor[address_choice]--; 


}

void
train_predictor_custom(uint32_t pc, uint8_t outcome)
{
	//int pc_lower_bits_custom = pc & (PER_LEN-1);
	int pc_lower_bits_custom = pc % PER_LEN;
	int bias = perceptron_table[pc_lower_bits_custom][0];
	int y; // dot product of history and perceptrons
	for(i=0;i<CUSTOM_HISTORY;i++)
		y = y+(perceptron_table[pc_lower_bits_custom][i+1]*custom_history[i]);
	y = y+bias;
	int outcome_int;
	outcome_int = (outcome == TAKEN) ? 1:-1;
	if((y>=0 && !outcome) || (y<0&&outcome) || (abs(y)<=THETA))
	{
	perceptron_table[pc_lower_bits_custom][0] = perceptron_table[pc_lower_bits_custom][0]+1*outcome_int;
		for(i=0;i<CUSTOM_HISTORY;i++)
			perceptron_table[pc_lower_bits_custom][i+1] = perceptron_table[pc_lower_bits_custom][i+1] + custom_history[i]*outcome_int;
	}
	int hist[CUSTOM_HISTORY];
	for(i=0;i<CUSTOM_HISTORY;i++)
	{
		hist[i] = custom_history[i];
			if(i==0)
			{
			custom_history[0] = hist[0];
		}
		else
		{
		 	custom_history[i] = hist[i-1]; 
		}
	}
		custom_history[0] = (outcome == TAKEN) ? 1:-1;
}



void train_predictor(uint32_t pc, uint8_t outcome)
{
  switch (bpType) {
    case STATIC:
    case GSHARE:
      train_predictor_gshare(pc,outcome);
      break;
      printf("is this called");
    case TOURNAMENT:
      train_predictor_tournament(pc,outcome);
      break;
    case CUSTOM:
      train_predictor_custom(pc,outcome);
      break;
    default:
      break;
  }
}






