#!/usr/bin/perl -w

use strict;
use warnings FATAL => qw(uninitialized);

use Symbol;

use threads;
use threads::shared;
# use Thread::Queue;
 
# Subroutine that starts workers
sub create_workers;

# Subroutines for building forest
sub build_forest;
sub add_service;
sub add_host_to_service;
sub add_dependee_to_service;
sub add_referer_to_service;

# Generic routine for deleting from array
sub delete_from_array;

# Subroutines for checking for cycles
sub check_forest_for_cycles;
sub check_path_for_cycles;

# Subroutines for starting services
sub service_is_alive;
sub start_services;
sub start_tree;

# Subroutines for monitoring services
sub monitor_services;
sub monitor_tree;

# Subroutines for stopping services
sub stop_services;
sub stop_tree;

# Subroutines for statistics gathering and printing
sub statistics_add($$);
sub statistics_print();
sub forest_dependencies_print;

my $usage = 
    "Usage: manager.pl [-v] [-s] [--workers <number_of_workers>] " .
    "[--timeout <timeout_in_seconds>] [-f <config_file>] " .
    "[--service <service_name_pattern>] [--host <host_name>] " .
    "start | stop | park | status | list | deps [short]\n";

my $NOT_USE_SERVICE_MESSAGE = 0;
my $USE_SERVICE_MESSAGE = 1;

(@ARGV >= 1) or die $usage;

#
# Manager configuration (should be updated in map file)
#

our %service_locations;

#
#
#

my $map_file;
my $action = "start";
my $workers_num = 20;
my $verbose = 0;
my $timeout = 0;
my $gather_statistics = 0;
my %statistics :shared;
my $specific_host;
my $specific_service;
my $specific_action;
my $call_is_alive = 1;
my $short_dependencies = 0;

for (my $i = 0; $i < @ARGV; $i++)
{
  if ($ARGV[$i] eq "-f")
  {
    die $usage unless ($i + 1 < @ARGV);
    $i++;
    $map_file = $ARGV[$i];
  }
  elsif ($ARGV[$i] eq "-v")
  {
    $verbose = 1;
  }
  elsif ($ARGV[$i] eq "--workers")
  {
    die $usage unless ($i + 1 < @ARGV);
    $i++;
    $workers_num = $ARGV[$i];
  }
  elsif ($ARGV[$i] eq "--timeout")
  {
    die $usage unless ($i + 1 < @ARGV);
    $i++;
    $timeout = $ARGV[$i];
  }
  elsif ($ARGV[$i] eq "--service")
  {
    die $usage unless ($i + 1 < @ARGV);
    $i++;
    $specific_service = $ARGV[$i];
  }
  elsif ($ARGV[$i] eq "--host")
  {
    die $usage unless ($i + 1 < @ARGV);
    $i++;
    $specific_host = $ARGV[$i];
  }
  elsif ($ARGV[$i] eq "-s")
  {
    $gather_statistics = 1;
  }
  elsif ($ARGV[$i] eq "start")
  {
    $action = $ARGV[$i];
    last;
  }
  elsif ($ARGV[$i] eq "stop")
  {
    $action = $ARGV[$i];
  }
  elsif ($ARGV[$i] eq "park")
  {
    $action = $ARGV[$i];
  }
  elsif ($ARGV[$i] eq "status")
  {
    $action = $ARGV[$i];
  }
  elsif ($ARGV[$i] eq "list")
  {
    $call_is_alive = 0;
    $action = $ARGV[$i];
  }
  elsif ($ARGV[$i] eq "deps")
  {
    $action = $ARGV[$i];
    if ($i + 1 < @ARGV and $ARGV[$i + 1] eq "short")
    {
      $short_dependencies = 1;
      $i++;
    }
  }
  else
  {
    die $usage;
  }
}

if (!defined($map_file))
{
  $map_file = "locations.pl";
}

eval('require "' . $map_file . '"') or die "Error: can not load map file.\n";

my %service_forest;
build_forest();
check_forest_for_cycles();

our @workers;
our @task_queue : shared;
our $tasks_todo : shared = 0;
our $tasks_result : shared = 0;

my $exit_code = 0; # script exit code

if ($action eq "start")
{
  create_workers();
  $exit_code = start_services(); # !=0, if one or more services not started
  statistics_print();
  finalize_workers();
}
elsif ($action eq "stop")
{
  create_workers();
  stop_services(0);
  statistics_print();
  finalize_workers();
}
elsif ($action eq "park")
{
  create_workers();
  stop_services(1);
  finalize_workers();
}
elsif ($action eq "status" or $action eq "list")
{
  monitor_services();
}
elsif ($action eq "deps")
{
  forest_dependencies_print();
}

exit $exit_code;

#
# Subroutines
#

sub create_workers
{
  for (my $i = 0; $i < $workers_num; $i++)
  {
    $workers[$i] = threads->create(\&worker_proc);
  }
}

sub finalize_workers
{
  {
    lock $tasks_result;
    $tasks_result = 1;
    cond_broadcast($tasks_result);
  }

  foreach my $worker (@workers)
  {
    $worker->join();
  }
}

sub worker_proc
{
  my $task;
  while (1)
  {
    {
      lock $tasks_result;
      while (!$tasks_result && (@task_queue == 0))
      {
        cond_wait($tasks_result);
      }
      
      if ($tasks_result)
      {
        return;
      }
      
      $task = pop @task_queue;
    }

    # make a call
    eval($task);

    {
      lock $tasks_result;

      if ($@)
      {
        print($@);
        $tasks_result = 1;
        cond_broadcast($tasks_result);
        return;
      }

      $tasks_todo--;
      if ($tasks_todo == 0)
      {
        cond_broadcast($tasks_result);
      }
    }
  }
}

sub build_forest
{
  if (!defined(%service_locations))
  {
    die "Error: service_locations variable is not defined.";
  }
  
  while (my ($host, $services) = each(%service_locations))
  {
    foreach my $service (@$services)
    {
      add_service($service, $host, undef);
    }
  }
}

sub add_service
{
  my ($service_name, $host, $referer) = @_;

  if (!exists($service_forest{$service_name}))
  {
    if (!eval("require $service_name"))
    {
      die("Error: can not load '" . $service_name . 
                "' service module. ", $@);
    }
    $service_forest{$service_name} =
      {
        name      => $service_name, 
        incoming  => [],
        outgoing  => [],
        hosts     => []
      };

    my $depends_on = eval('\@' . $service_name . '::depends_on');
    foreach my $dependee (@$depends_on)
    {
      add_dependee_to_service($dependee, $service_name);
    }
  }

  if (defined($referer))
  {
    add_referer_to_service($referer, $service_name);
  }

  if (defined($host))
  {
    add_host_to_service($host, $service_name);
  }
}

sub add_host_to_service
{
  my ($host, $service_name) = @_;
  my $host_list = $service_forest{$service_name}{"hosts"};

  my $multiplicity = $${qualify_to_ref multiplicity => $service_name}
      || "1+";

  if ($multiplicity eq "1")
  {
    (@$host_list == 0) or 
        die("Error: there shouldn't be more that one host " . 
      "for $service_name. Odd location is $host.\n");
  }
  elsif ($multiplicity eq "1+")
  {}
  else
  {
    print("Warning: invalid value for multiplicity in $service_name " . 
          "service definition\n");
  }
    
  foreach my $host_in_list (@$host_list)
  {
    return if $host_in_list eq $host;
  }
  $host_list->[$#$host_list + 1] = $host;
}

sub add_dependee_to_service
{
  my ($dependee, $service_name) = @_;
  my $outgoing_list = $service_forest{$service_name}{"outgoing"};
  foreach my $dependee_in_list (@$outgoing_list)
  {
    if ($dependee_in_list eq $dependee)
    {
      print("warning: duplicate entry detected in 'depends_on' list of '" 
        . $service_name . "' service.");
      return;
    }
  }
  $outgoing_list->[$#$outgoing_list + 1] = $dependee;
  add_service($dependee, undef, $service_name);
}

sub add_referer_to_service
{
  my ($referer, $service_name) = @_;
  my $incoming_list = $service_forest{$service_name}{"incoming"};
  foreach my $referer_in_list (@$incoming_list)
  {
    return if ($referer_in_list eq $referer);
  }
  $incoming_list->[$#$incoming_list + 1] = $referer;
}

sub delete_from_array
{
  my ($array_ref, $idx) = @_;

  return if ($idx > @$array_ref);
  if ($idx < $#$array_ref)
  {
    $$array_ref[$idx] = $$array_ref[$#$array_ref];
  }
  $#$array_ref--;
}

sub print_hash
{
  my ($hash) = @_;
  my $prefix = "";
  my $result = "";
  foreach my $node (keys %{$hash})
  {
    $result = $result . $prefix . $node;
    $prefix = ", ";
  }
  if ($result eq "")
  {
    $result = "???";
  }
  print($result);
}

sub forest_dependencies_print
{
  my %monitored_services;
  while (my ($service_name, $service_def) = each(%service_forest))
  {
    if (@{$service_def->{"incoming"}} == 0)
    {
      my %result;
      eval { %result =
        path_dependencies_print($service_name, \%monitored_services, 1); };
      if ($@)
      {
        print("Failed. " . $@ . "\n");
        return 1;
      }
    }
  }
}

sub path_dependencies_print
{
  my ($service_name, $vertexes, $dep) = @_;
  my %own_dependencies;

  if (!exists($vertexes->{$service_name}))
  {
    foreach my $child (@{$service_forest{$service_name}->{"outgoing"}})
    {
      # add dependencies of nodes
      my %node_deps;
      %node_deps = path_dependencies_print($child, $vertexes, $dep + 1);
      if (!$short_dependencies)
      {
        while (my ($key, $value) = each(%node_deps))
        {
          $own_dependencies{$key} = $value;
        }
      }
      # direct depends add

      if ((not defined($specific_service) or
           $service_name =~ /$specific_service/) and
           @{$service_forest{$service_name}->{"hosts"}} > 0)
      {
        foreach my $host (@{$service_forest{$service_name}->{"hosts"}})
        {
          my $type = eval('$' . $service_name . '::type');
          if ($@)
          {
            die("Error occured while determining $service_name type. " 
                . "Description: " . $@ . "\n");
          }

          if ($type ne "SERVICE" && $type ne "TASK")
          {
            die("Unexpected type '" . $type . "' of $service_name.\n");
          }

          if ($type eq "SERVICE" and (!defined($specific_host) or
            $host eq $specific_host))
          {
            $own_dependencies{$child}{$host} = $host;
          }
        }
      }
    }

    $vertexes->{$service_name} = 1;
    # hosts list for $service_name take from forest, hosts for depends
    # stored in %own_dependencies
    my $host_list = $service_forest{$service_name}{"hosts"};
    my %host_hash;
    foreach my $node (@$host_list)
    {
      $host_hash{$node} = $node;
    }
    
    if (defined($specific_host) and !exists($host_hash{$specific_host}))
    {
      return %own_dependencies;
    }
    print("$service_name on ");
        
#    print_hash(%host_hash);
    my $prefix = "";
    my $result = "";
    foreach my $node (@$host_list)
    {
      $result = $result . $prefix . $node;
      $prefix = ", ";
    }
    if ($result eq "")
    {
      $result = "???";
    }
    print($result);
    if (scalar keys %own_dependencies ne 0)
    { # print nodes with depends
      print(" depends");
      while (my ($key, $value) = each(%own_dependencies))
      {
        print("\n  $key on ");
        print_hash($value);
      }
    }
    else
    {
      print(" independent");
    }
    print("\n\n");
  }
  return %own_dependencies;
}

sub check_forest_for_cycles
{
  while (my ($service_name, $service_def) = each(%service_forest))
  {
    if (@{$service_def->{"incoming"}} == 0)
    {
#     print("Checking path for cycles: " . $service_name . "\n");
      my %vertexes = ();
      check_path_for_cycles($service_name, \%vertexes);
    }
  }
}

sub check_path_for_cycles
{
  my ($service_name, $vertexes) = @_;
# print($service_name);
  if (exists($vertexes->{$service_name}))
  {
    die("Error: there is cyclic dependency in configuration. " . 
       "Returned to $service_name while walking through the forest.");
  }
  $vertexes->{$service_name} = 1;

  my $child;
# print(" (");
# foreach $child (@{$service_forest{$service_name}{"outgoing"}})
# {
#   print($child . ", ");
# }
# print(")\n");

  foreach $child (@{$service_forest{$service_name}{"outgoing"}})
  {
    check_path_for_cycles($child, $vertexes);
  }
  delete($vertexes->{$service_name});
}

sub service_is_alive
{
  my ($service_name, $host, $verbose, $non_alive_message) = @_;

  my $alive = 0;
  my $message = $non_alive_message;

  my $is_alive_result = ${qualify_to_ref is_alive => $service_name}->($host, $verbose);

  if ($@)
  {
    die "Error occured while calling ${service_name}::is_alive: $@";
  }

  if (ref($is_alive_result) eq "HASH")
  {
    $alive = $is_alive_result->{status};
    
    if ($message)
    {
      $message = "$message : " . $is_alive_result->{message};
    }
  }
  else
  {
    $alive = $is_alive_result;
  }

  if (!$alive && $message)
  {
    print "$message\n";
  }

  return $alive;
}

sub start_services
{
  my %started_services;
  while (my ($service_name, $service_def) = each(%service_forest))
  {
    if (@{$service_def->{"incoming"}} == 0)
    {
      eval { start_tree($service_name, \%started_services); };
      if ($@)
      {
        print("Failed to start services. See above errors.\n");
        return 1;
      }
    }
  }
  return 0;
}

sub start_tree
{
  my ($service_name, $started_services) = @_;
  if (!exists($started_services->{$service_name}))
  {
    foreach my $child (@{$service_forest{$service_name}->{"outgoing"}})
    {
      start_tree($child, $started_services);
    }

    if ((not defined($specific_service) or
        $service_name =~ /$specific_service/) and
      @{$service_forest{$service_name}->{"hosts"}} > 0)
    {
      lock($tasks_result);

      $tasks_todo = 0;
      foreach my $host (@{$service_forest{$service_name}->{"hosts"}})
      {
        next if defined($specific_host) and $host ne $specific_host;
        push(@task_queue, 'start_service_on_host("' . 
             $service_name . '", "' . 
             $host . '")');
        $tasks_todo++;
        $specific_action = 1;
      }

      # waking up workers for processing
      cond_broadcast($tasks_result);
      
      # monitor $tasks_result and queue size
      while (!$tasks_result && ($tasks_todo != 0))
      {
        cond_wait($tasks_result);
      }

      if ($tasks_result)
      {
        die "Error";
      }
    }

    $started_services->{$service_name} = 1;
  }
}

sub start_service_on_host
{
  my ($service_name, $host) = @_;
  print "Starting $service_name on $host\n";

  my $type = $${qualify_to_ref type => $service_name} || '(undef)';

  if ($type ne "SERVICE" && $type ne "TASK")
  {
    die("Unexpected type '" . $type . "' of $service_name.\n");
  }

  if($type eq "SERVICE")
  {
    if(service_is_alive($service_name, $host, $verbose))
    {
      print "$service_name has been already started on $host\n";
      return;
    }
  }

  my $action_start = time();

  ${qualify_to_ref start => $service_name}->($host, $verbose);
  # if ($@)
  # {
  #   die("Error occured while starting $service_name on $host. " 
  #       . "Description: " . $@ . "\n");
  # }

  if($type eq "SERVICE")
  {
    my $not_ready_message = "Service $service_name is not ready on $host";
    my $stop = $timeout > 0 ? time() + $timeout : 0;
    sleep(1);

    while (1)
    {
      if(service_is_alive(
           $service_name,
           $host,
           $verbose,
           $not_ready_message))
      {
        statistics_add($service_name, time() - $action_start);
        print "$service_name started on $host\n";
        return;
      }
      else
      {
        die "Service $service_name is not ready on $host in $timeout seconds.\n"
          if $stop && time() >= $stop;
        sleep(2);
      }
    }
  }
}

sub monitor_services
{
  my %monitored_services;
  while (my ($service_name, $service_def) = each(%service_forest))
  {
    if (@{$service_def->{"incoming"}} == 0)
    {
      eval { monitor_tree($service_name, \%monitored_services); };
      if ($@)
      {
        print("Failed. " . $@ . "\n");
        return 1;
      }
    }
  }
}

sub monitor_tree
{
  my ($service_name, $monitored_services) = @_;
  if (!exists($monitored_services->{$service_name}))
  {
    foreach my $child (@{$service_forest{$service_name}->{"outgoing"}})
    {
      monitor_tree($child, $monitored_services);
    }

    foreach my $host (@{$service_forest{$service_name}->{"hosts"}})
    {
      my $type = eval('$' . $service_name . '::type');

      if ($@)
      {
        die("Error occured while determining $service_name type. " 
            . "Description: " . $@ . "\n");
      }

      if ($type ne "SERVICE" && $type ne "TASK")
      {
        die("Unexpected type '" . $type . "' of $service_name.\n");
      }

      if ($type eq "SERVICE")
      {
        if ($call_is_alive)
        {
          my $not_alive_message = "$service_name is not alive on $host";

          if (service_is_alive(
               $service_name,
               $host,
               $verbose,
               $not_alive_message))
          {
            print "$service_name is alive on $host\n";
          }
        }
        else
        {
          print "Service: $service_name is standing on $host\n";
        }
      }
    }

    $monitored_services->{$service_name} = 1;
  }
}

sub stop_services
{
  my ($park) = @_;

  my %stopped_services;
  while (my ($service_name, $service_def) = each(%service_forest))
  {
    if (@{$service_def->{"outgoing"}} == 0)
    {
      eval { stop_tree($service_name, \%stopped_services, $park); };
      if ($@)
      {
        print("Failed to start services. See above errors.\n");
        return 1;
      }
    }
  }
}

sub stop_tree
{
  my ($service_name, $stopped_services, $park) = @_;
  if (!exists($stopped_services->{$service_name}))
  {
    foreach my $child (@{$service_forest{$service_name}->{"incoming"}})
    {
      stop_tree($child, $stopped_services, $park);
    }

    if ((not defined($specific_service) or
        $service_name =~ /$specific_service/) and
      @{$service_forest{$service_name}->{"hosts"}} > 0)
    {
      lock($tasks_result);

      $tasks_todo = 0;
      foreach my $host (@{$service_forest{$service_name}->{"hosts"}})
      {
        next if defined($specific_host) and $host ne $specific_host;
        push(@task_queue, 'stop_service_on_host("' . 
             $service_name . '", "' . 
             $host . '", ' . $park . ')');
        $tasks_todo++;
        $specific_action = 1;
      }
      
      # waking up workers for processing
      cond_broadcast($tasks_result);
      
      # monitor $tasks_result and queue size
      while (!$tasks_result && ($tasks_todo != 0))
      {
        cond_wait($tasks_result);
      }

      if ($tasks_result)
      {
        die "Error";
      }
    }

    $stopped_services->{$service_name} = 1;
  }
}

sub stop_service_on_host
{
  my ($service_name, $host, $park) = @_;

  print (($park ? "Parking" : "Stopping") . " $service_name on $host\n");

  my $type = $${qualify_to_ref type => $service_name} || '(undefined)';

  if ($type ne "SERVICE" && $type ne "TASK")
  {
    die("Unexpected type '" . $type . "' of $service_name.\n");
  }

  my $action_start = time();

  if($type eq "SERVICE")
  {
    my $not_started_message = "$service_name is not started on $host";

    if(!service_is_alive(
         $service_name,
         $host,
         $verbose,
         $not_started_message))
    {
      my $term_func = $service_name . '::terminate';
      if(defined(&$term_func))
      {
        my $none = eval($service_name . '::terminate' . '($host, $verbose)');
        if(!$@ && $none)
        {
          print "$service_name terminated on $host \n";
          return;
        }
      }
      return;
    }
  }

  &${qualify_to_ref $park ? 'park' : 'stop' => $service_name} ($host, $verbose);

  if($type eq "SERVICE")
  {
    sleep(1);
    for (my $i = 0; $i < 5; )
    {
      if(!service_is_alive($service_name, $host, $verbose))
      {
        statistics_add($service_name, time() - $action_start);
        print "$service_name " . ($park ? "parked" : "stopped") . 
              " on $host\n";
        return;
      }
      else
      {
        print "Service $service_name is still running on $host. Waiting...\n";
        sleep(5);
      }

      if(!$park)
      {
        $i++;
      }
    }
  
    print("Service $service_name is still running on $host. Giving up.\n");
  }
}

sub statistics_add($$)
{
  if ($gather_statistics)
  {
    lock(%statistics);
    $statistics{$_[0]} = $_[1];
  }
}
sub statistics_print()
{
  if (defined($specific_service) or defined($specific_host))
  {
    print("No actions have been performed for specific service name " .
      "and/or specific host name") unless defined($specific_action);
  }

  lock(%statistics);
  foreach (sort keys %statistics)
  {
    print $_, " => ", $statistics{$_}, " seconds\n";
  }
}
