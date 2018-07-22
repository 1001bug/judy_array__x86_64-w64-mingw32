// @(#) $Revision: 4.5 $ $Source: /judy/src/apps/demo/funhist.c $
//
// FUNCTION HISTOGRAM; JUDYL DEMO PROGRAM
//
// This program uses JudyL to create a histogram of data generated by the
// function random(3M).  Other functions could be substituted in.
//
// The program generates n random numbers, stores them in a JudyL array
// and then prints a histogram.  All aspects of the generation and
// histogram are timed so the user can see how intptr_t various JudyL functions
// take.
//
//
// This demonstrates:
//           JLI        (JudyLIns)
//           JLF        (JudyLFirst)
//           JLL        (JudyLLast)
//           JLN        (JudyLNext)
//           JLC        (JudyLCount)
//           JLBC       (JudyLByCount)
//           how to access a JudyL value.
//
// Notice that using JudyL gives you fast stores and lookups as with hashing
// but without having to define a hash function, initialize the hash table
// or having to predetermine a good hash table size.  It also gives you a
// sorted list at the same time.
// Also notice that JudyLCount is much faster than you can sequentially
// count items in an array.
//

#include <stdlib.h>     // random()
#include <stdio.h>      // printf()
#include <sys/time.h>   // gettimeofday()
#include <Judy.h>       // JL*() routines

// Default number of iterations (number of random numbers generated)
// This may be overridden on the command line

#define DEFAULT_ITER 1000000

// The number of buckets the histogram is divided into.

#define DEFAULT_HISTO_BUCKETS 32

// Macro for correction english output for plurals

#define	PLURAL(count) ((count == 1) ? "" : "s")

// timing routines

struct timeval TBeg, TEnd;
#define STARTTm         gettimeofday(&TBeg, NULL)
#define ENDTm           gettimeofday(&TEnd, NULL)
#define DeltaUSec                                       \
        ((double)(TEnd.tv_sec - TBeg.tv_sec) * 1E6 +    \
         (TEnd.tv_usec - TBeg.tv_usec))

// End of timing routines

int
main(int argc, char **argv)
{
    Pvoid_t   PJArray = NULL;   // main JudyL array
                                // key is random number, value is repeat count
    Pvoid_t   PJCount = NULL;   // second JudyL array
                                // key is repeat count (from PJArray)
                                // value is count of items with the same
                                // repeat count
    Word_t    Index;            // index in JudyLFirst/Next loop
    PWord_t   PValue;           // pointer to Judy array value
    PWord_t   PGenCount;        // pointer to generation count
    Word_t    num_vals;         // number of randoms to generate
    Word_t    iter;             // for loop iteration
    Word_t    unique_nums;      // number of unique randoms generated
    Word_t    random_num;       // random number
    Word_t    median;           // random number
    Word_t    tmp1, tmp2;       // temporary variable
    double    random_gen_time;  // time to generate random numbers
    Word_t    histo_incr;       // histogram increment
    uintptr_t ran_sum = 0; // sum of all generated randoms

// TITLE

    printf("\nJudyL demonstration: random(3M) histogram\n");

// SET NUMBER OF RANDOMS TO GENERATE

    if (argc != 2)
    {
        // SET TO DEFAULT_ITER
        num_vals = DEFAULT_ITER;
        printf("Usage: %s [numvals]\n", argv[0]);
        printf("  Since you did not specify a number of values, %"PRIdPTR"\n",
               num_vals);
        printf("  will be used as the number of random numbers to insert\n");
        printf("  into the Judy array\n");
    }
    else
    {
// OVERRIDE NUMBER OF RANDOMS TO GENERATE

        num_vals = atol(argv[1]);
    }

// TIME THE GENERATION OF ALL THE RANDOM NUMBERS.  THIS TIME IS LATER
//
// This time is later subtracted from the insert loop time so that the
// time it takes to do the actual JudyLIns can be isolated from the
// total time.

    printf("\ntiming random number generation\n");
    STARTTm;
    for (iter = num_vals; iter; iter--)
    {
        random();
    }                           /* end of random number generator time */
    ENDTm;
    random_gen_time = DeltaUSec;
    printf("It took %.3f sec to generate %"PRIdPTR" random numbers\n",
           random_gen_time / 1000000, num_vals);
    printf("  (ie. %.3f uSec/number)\n\n", random_gen_time / num_vals);

// REGENERATE RANDOMS AND INSERT THEM INTO A JUDYL ARRAY

    printf("Please wait while the random numbers are inserted into\n");
    printf("a JudyL array (with a usage count) ...\n");

    STARTTm;
    for (iter = num_vals; iter; iter--)
    {
        random_num = (Word_t)random();

        JLI(PValue, PJArray, random_num);

        /* increment hit count */
        (*PValue)++;

        /* sum the random number */
        ran_sum += random_num;
    }                           /* end of random number generator time */
    ENDTm;
    printf("That took %.3f uSec/Index.\n",
           (DeltaUSec - random_gen_time) / num_vals);

// COUNT THE NUMBER OF ELEMENTS IN THE JUDYL ARRAY
// IE. COUNT THE NUMBER OF UNIQUE RANDOM NUMBERS

    JLC(unique_nums, PJArray, 0, -1);
    printf("\nThere were %"PRIdPTR" unique random numbers generated\n", unique_nums);

// FIND HOW MANY NUMBERS WERE GENERATED ONCE, TWICE, ...
//
// Create a new JudyL array where the index is the count from PJArray
// and the value is a count of the number of elements with that count.

    if (unique_nums != num_vals)
    {
        printf("\nLooping through the entire Judy array to create a\n");
        printf("new Judy counting array (PJCount in the source code)\n");
        printf("...\n");

        STARTTm;
        Index = 0;
        JLF(PValue, PJArray, Index);
        while (PValue != (PWord_t)NULL)
        {
            JLI(PGenCount, PJCount, *PValue);

            (*PGenCount)++;     // increment hit count

            JLN(PValue, PJArray, Index);
        }
        ENDTm;

        printf("That took %.3f Secs or %.3f uSec/Index\n\n",
               DeltaUSec / 1000000, DeltaUSec / unique_nums);

// PRINT DUPLICATES HISTOGRAM

        printf("Duplicates Histogram:\n");

        Index = 0;
        JLF(PValue, PJCount, Index);
        while (PValue != (PWord_t)NULL)
        {
            printf("  %"PRIdPTR" numbers were generated %"PRIdPTR" time%s\n",
                   *PValue, Index, PLURAL(Index));

            JLN(PValue, PJCount, Index);
        }
    }

// PRINT DISTRIBUTION HISTOGRAM

    printf("\nCompute the random number distribution by counting index ranges\n");

    histo_incr = ((Word_t)~((Word_t)0) / DEFAULT_HISTO_BUCKETS) >> 1;

    Index = ((Word_t)0);
    for (iter = 0; iter < DEFAULT_HISTO_BUCKETS; iter++)
    {
        Word_t    Count;

        JLC(Count, PJArray, Index, Index + histo_incr);
        printf("  %"PRIdPTR" unique values from 0x%08"PRIxPTR" - 0x%08"PRIxPTR"\n", Count,
               Index, Index + histo_incr);

        Index += histo_incr + 1;

    }

// PRINT MEAN (average),
//       MEDIAN (middle value, or average of two middle values)
//       RANGE (low and high value)

    tmp1 = (Word_t)(ran_sum / (long long)num_vals);
    printf("                  mean: 0x%"PRIxPTR"\n", tmp1);

// If there were an even number of randoms generated, then average
// the two middle numbers.  Otherwise, the mean is the middle value
    if (num_vals & 1)
    {
        JLBC(PValue, PJArray, num_vals / 2, tmp1);
        JLBC(PValue, PJArray, (num_vals / 2) + 1, tmp2);
        median = (tmp1 + tmp2) / 2;
    }
    else
    {
        JLBC(PValue, PJArray, (num_vals + 1) / 2, median);
    }
    printf("                median: 0x%"PRIxPTR"\n", median);

    Index = 0;
    JLF(PValue, PJArray, Index);
    printf("first random generated: 0x%"PRIxPTR"\n", Index);

    Index = ~0;
    JLL(PValue, PJArray, Index);
    printf(" last random generated: 0x%"PRIxPTR"\n", Index);

    return (0);

}                               // main()
