#!/usr/bin/perl

my $usage = "Usage: $0 <input file name> <output file name> [<output environment file>]";

(scalar(@ARGV) == 2) or (scalar(@ARGV) == 3) or print_error_and_exit($usage);

my ($input_file, $output_file, $output_vars_file) = @ARGV;

open(SOURCE, "< $input_file")
    or print_error_and_exit("Couldn't open $input_file for reading: $!");

open(DEST, "> $output_file")
    or print_error_and_exit("Couldn't open $output_file for writing: $!");

my %output_vars = {};

if(defined($output_vars_file))
{
  %output_vars = parse_vars($output_vars_file);
}

my $line_no = 0;

while (<SOURCE>)
{
  $line_no++;

  s|\$\$|\a|g;

  # substitute variables
  while (m/\$\(((\w*)(?:\=(?:(\'(?:[^\'\\]*|\\[^\']|\\\')*\')|(\w*))\s*)?)\)/)
  {
    my $expr = $1; 
    my $var_name = $2;
    my $uc_var_name = $var_name;
    my $var_value;

    if (defined($3))
    {
      $var_value = $3;
      $var_value =~ s|\\\\|\\|g;
      $var_value =~ s|\\\'|\'|g;
    }

    if (defined($4))
    {
      $var_value = $4;
    }

    if (defined($var_value))
    {
      $output_vars{$uc_var_name} = $var_value;
      $ENV{$uc_var_name} = $var_value;
      $var_value =~ s|\$|\a|g;
    }
    
    if (exists($ENV{$uc_var_name}))
    {
      $var_value = $ENV{$uc_var_name};
    }
    else
    {
      print_error_and_exit("$input_file at line $line_no: Variable \"$var_name\" is not set");
    }
      
    $var_value =~ s|\$|\a|g;
    s|\$\($expr\)|$var_value|g;
  }

  if (/\$/)
  {
    print_error_and_exit("$input_file at line $line_no: Syntax error. If you mean '\$', double it");
  }

  s|\a|\$|g;

  # define variables
  if (m|^\s*Set\s|)
  {
    if (m|^\s*Set\s+(\w*)\s+(.*)$|)
    {
      my $val = $2;
      $val =~ s|\s+$||;
      $ENV{$1} = $val;
    }
    else
    {
      print_error_and_exit("$input_file at line $line_no: Syntax error in Set directive");
    }
  }
  else
  {
    print DEST;
  }
}

close(SOURCE);
close(DEST);

if(defined($output_vars_file))
{
  save_vars($output_vars_file, %output_vars);
}

sub parse_vars
{
  my ($input_vars) = @_;

  my %defined_vars = {};

  if(open(INPUT_VARS, "< $input_vars"))
  {
    while(<INPUT_VARS>)
    {
      if(m/^\s*(\w*)\s*=(.*)$/)
      {
        $defined_vars{$1} = $2;
      }
      elsif(m/^\s*export\s*(\w*)\s*$/)
      {
        if(defined($defined_vars{$1}))
        {
          $exported_vars{$1} = $defined_vars{$1};
        }
      }
    }

    close(INPUT_VARS);
  }

  return %exported_vars;
}

sub save_vars
{
  my ($output_vars, %defined_vars) = @_;

  open(OUTPUT_VARS, "> $output_vars")
     or print_error_and_exit("Couldn't open $output_vars for writing: $!");

  while(($var_name, $var_value) = each(%defined_vars))
  {
    print OUTPUT_VARS "$var_name=$var_value\nexport $var_name\n\n";
  }

  close(OUTPUT_VARS);
}

sub print_error_and_exit($)
{
  print "$_[0]\n";
  exit 1;
}
