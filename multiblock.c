#include <stdio.h>
#include <time.h>
#include <argp.h>
#include <stdlib.h>

# ifndef MIN
# define MIN(x,y) ((x)<(y)?(x):(y))
# endif
# ifndef MAX
# define MAX(x,y) ((x)>(y)?(x):(y))
# endif

#ifndef BLOCK_SIZE
#   define BLOCK_SIZE (32)
#endif
#ifndef NUM_BLOCKS
#   define NUM_BLOCKS (100000)
#endif
#ifndef BLOCK_STRIDE
#   define BLOCK_STRIDE (1)
#endif
#ifndef INNER_REPS
#   define INNER_REPS (1)
#endif
#ifndef OUTER_REPS
#   define OUTER_REPS (10)
#endif
#ifndef BURN_REPS
#   define BURN_REPS (1)
#endif

/* This structure is used by main to communicate with parse_opt. */
struct args
{
  long block_size;
  long num_blocks;
  long block_stride;
  long inner_reps;
  long outer_reps;
  long burn_reps;
};

/*
   OPTIONS.  Field 1 in ARGP.
   Order of fields: {NAME, KEY, ARG, FLAGS, DOC}.
*/
static struct argp_option options[] =
{
  {"block_size",'b',"count",0,"Size of blocks of doubles (default 32)"},
  {"num_blocks",'n',"count",0,"Number of blocks (default 100000)"},
  {"block_stride",'s',"count",0,"Stride between blocks (default 1)"},
  {"inner_reps",'i',"count",0,"Number of inner reps (default 1)"},
  {"outer_reps",'o',"count",0,"Number of outer reps (default 10)"},
  {"burn_reps",'z',"count",0,"Number of outer reps to burn (default 1)"},
  {0}
};

/*
   PARSER. Field 2 in ARGP.
*/
static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
  struct args *args = state->input;
  switch (key)
    {
    case ARGP_KEY_INIT:
      args->block_size = BLOCK_SIZE;
      args->num_blocks = NUM_BLOCKS;
      args->inner_reps = INNER_REPS;
      args->outer_reps = OUTER_REPS;
      args->burn_reps = BURN_REPS;
      break;
    case 'b':
      args->block_size = strtol(arg,NULL,10);
      break;
    case 'n':
      args->num_blocks = strtol(arg,NULL,10);
      break;
    case 's':
      args->block_stride = strtol(arg,NULL,10);
      break;
    case 'i':
      args->inner_reps = strtol(arg,NULL,10);
      break;
    case 'o':
      args->outer_reps = strtol(arg,NULL,10);
      break;
    case 'z':
      args->burn_reps = strtol(arg,NULL,10);
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

// dirty gcd; does not check parameters
long gcd(long a, long b)
{
  long c;
  while (a) {
    c = a;
    a = b%a;
    b = c;
  }
  return b;
}


long clockloops(struct args *args, double *baseA, double *baseB)
{
  long block_size = args->block_size;
  long inner_reps = args->inner_reps;
  long num_blocks = args->num_blocks;
  long block_stride = args->block_stride;
  double *thisA;
  double *thisB;
  clock_t lasttick;
  long aind;
  long nind;
  long rind;

  // loop over each bloc
  lasttick = clock();
  for (nind=0; nind < num_blocks; ++nind) {
    // stride
    thisA = baseA+block_size*((nind*block_stride)%num_blocks);
    thisB = baseB+block_size*((nind*block_stride)%num_blocks);
    // loop over repetitions per block
    for (rind=0; rind < inner_reps; ++rind) {
      // loop over block (hopefully vectorizable!)
      for (aind=0; aind < block_size; ++aind) {
        // op: multiply and add
        thisA[aind] = thisA[aind] * thisB[aind] + 0.1;
      }
    }
  }
  lasttick = clock()-lasttick;
  
  return lasttick;
}


int main(int argc, char **argv)
{
  struct args args;
  double *baseA;
  double *baseB;
  long aind;
  long rind;
  clock_t thistick;
  clock_t tick_total;
  clock_t min_tick = INT_MAX;

  // argument parsing
  static struct argp argp = {options, parse_opt, NULL, NULL};
  argp_parse(&argp, argc, argv, 0, 0, &args);
  if (gcd(args.block_stride,args.num_blocks) != 1) {
    printf("block_stride divides num_blocks; increasing num_blocks to %ld\n",
           args.num_blocks+1);
    args.num_blocks += 1;
  }

  // memory allocations
  printf("malloc'ing %ld bytes\n",
         sizeof(double)*2*args.num_blocks*args.block_size);
  baseA = malloc(sizeof(double) * args.num_blocks * args.block_size);
  baseB = malloc(sizeof(double) * args.num_blocks * args.block_size);

  // initialization; stupid values
  for (aind=0; aind<args.num_blocks*args.block_size; ++aind) {
    baseA[aind] = 2.0+(double)(aind+1)/args.num_blocks/args.block_size;
    baseB[aind] = 1.0-(double)(aind+1)/args.num_blocks/args.block_size/args.inner_reps;
  }

  // loop
  tick_total = 0;
  for (rind=-args.burn_reps; rind < args.outer_reps; ++rind) {
    thistick = clockloops(&args,baseA,baseB);
    // burn first half
    if (rind < 0) {
      printf("%ld burnin seconds: %f\n",rind,(double)thistick/CLOCKS_PER_SEC);
    }
    else {
      printf("%ld outer rep seconds: %f\n",rind,(double)thistick/CLOCKS_PER_SEC);
      tick_total += thistick;
      min_tick = MIN(min_tick,thistick);
    }
  }

  // print results
  printf("min seconds: %f\n",(double)min_tick/CLOCKS_PER_SEC);
  printf("max ops/second: %f\n",(double)args.num_blocks*args.inner_reps*args.block_size/min_tick*CLOCKS_PER_SEC);
}
