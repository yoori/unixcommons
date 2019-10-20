#!/usr/bin/perl

my @segmentors = (klt, mecab, polyglot, composite, deep, deepcheck);

#Stress test
my @threads = (10, 20);
foreach $thread (@threads)
{
  foreach $segmentor (@segmentors)
  {
    my $command = "~/projects/unixcommons/trunk/build/tests/Language/SegmentorCommonTests/PerformanceTest/SegmentorPerformanceTest ".
      "-i 100000000 -t $thread -s $segmentor ~/projects/unixcommons/trunk/tests/Language/Data/ ".
      "1>stress_${segmentor}_${thread}.out 2>stress_${segmentor}_${thread}.err";

    print "run command '$command'\n";
    print `$command`;
  }
}

#Ling test
foreach $segmentor (@segmentors)
{
  my $command = "~/projects/unixcommons/trunk/build/tests/Language/SegmentorCommonTests/PerformanceTest/SegmentorPerformanceTest ".
    "-a -t 1 -s $segmentor ~/projects/unixcommons/trunk/tests/Language/Data/ ".
    "1>ling_${segmentor}.out 2>ling_${segmentor}.err";

  print "run command '$command'\n";
  print `$command`;
}

my $custom_string = "\226\128\178RRRR\226\128\178RRRR\226\128\178 ".
                    "\226\128\198RRRR\226\128\198RRRR\226\128\198 ".
                    "\226\158\178RRRR\226\158\178RRRR\226\158\178 ";
#Custom test
foreach $segmentor (@segmentors)
{
  my $command = "echo \"$custom_string\" | ~/projects/unixcommons/trunk/build/tests/Language/SegmentorCommonTests/PerformanceTest/SegmentorPerformanceTest ".
  "-c -a -t1 -s $segmentor 1>custom_${segmentor}.out 2>custom_${segmentor}.err\n";

  print "run command '$command'\n";
  print `$command`;
}

#Valgrind test
foreach $segmentor (@segmentors)
{
  my $command = "valgrind -q --tool=memcheck --leak-check=full --num-callers=50 --suppressions=\${HOME}/projects/unixcommons/trunk/tests/Test.supp ".
    "\${HOME}/projects/unixcommons/trunk/build/tests/Language/SegmentorCommonTests/PerformanceTest/SegmentorPerformanceTest -t 3 -s $segmentor ".
    "1>valg_${segmentor}.out 2>valg_${segmentor}.err\n";

  print "run command '$command'\n";
  print `$command`;
}
