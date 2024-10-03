Linux-only project as it uses unistd.h

Results:
As the copy buffer size increased, the run time of the copy program decreased.
Specifically, between 16B and 256B, the run time would approximate halve as the
copy buffer size doubled.
This trend stopped at about 1024B as the decrease in run time became marginal 
even though the copy buffer size continued to double.
