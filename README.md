# HPSC Lab 2
2019-09-06

In the last lab we looked at effects of different memory access locations and patterns.  This week we are adding processing loads into the mix.  The goals are:
* Be able to differentiate memory-bound and processing-bound workloads for a system in practice.
* Observe some effects of vectorization and learn programming techniques to help the compiler vectorize.
* Produce evidence of critical thinking.  In particular, for each of the next two sections, **create one or two labeled charts** of something you explored, along with at most two paragraphs of explanations or other observations that you found helpful or interesting.  You may write in a [Markdown](https://guides.github.com/features/mastering-markdown/) file `Report.md` or a Jupyter notebook `Report.ipynb`.

WORDS OF CAUTION:
As encountered in the last lab, if you are using a shared resource (even if you are only sharing it with yourself), the OS Kernel may be unkind to you.  Machines may also be prone to turning components off while idling, heating up, and/or throttling themselves.  Be wary!  Run speed tests multiple times with pauses (not only in succession), and convince yourself whether (or not) the results are reasonable.

-----

This lab uses the code multiblock.c.  Similar to stream.c, it creates arrays and performs multiplications between them.  It has additional parameters to control inner and outer repetitions, as well as an inner "block" size that we can vectorize.

Compile with -O2.  Scan block_size (with num_blocks=1 and sufficient inner_reps) to try and find the cache transitions as in the previous lab.

Then with block_size=32 and num_blocks so that the total number of bytes gives rates in the plateau of a fast cache, exponentially increase inner_reps until you stop seeing improvement in ops/second.  This is a processing-bound regime; the machine is only capable of a bounded ops/second.

How does the change in ops rate compare to the roofline model for lower values of inner_reps (which controls "operational intensity")?  This is the memory-bound regime.

Neat!  Are other groups in the lab finding similar things?

-----

Go back to the earliest processing-bound parameters.  Now add these flags to the compile line (touch the .c file and remake to see the default compile line), and optionally output to a different file (`-o multiblock_vec`):

    -ftree-vectorize -fopt-info-vec-optimized

You "should" be given notices from the `-fopt-info-vec-optimized` flag about the inner loop being vectorized.  Rerun with the earliest processing-bound parameters you found above.  You should see a performance improvement.  Now if you double inner_reps you should see still more improvement.  This means that the processing saturation point has shifted!  Hopefully you are curious to characterize this new roofline and see what it looks like as well.

What is the relationship between the vectorized and unvectorized versions as you change block_size?

-----

Still looking for more?  Try repeating the characterizations for different cache uses?  How are they similar/different?

If you want to get your hands on more coding, multiblock.c currently has a simple multiply inside the innermost loop.  You could try other kernels.  Which can be vectorized?  Can you come up with any that you can fuse with additional compiler flags, or noticeably leverage other performance features such as simultaneous multi-threading?
