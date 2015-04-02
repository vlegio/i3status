#include <stdio.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

#define ToGB(bytes) (bytes/(1024.0*1024.0*1024.0))
#define DEBUG 1

int read_int_from_file(char filename[]){
  FILE *file;
  int res = 0;
  file = fopen(filename, "rt");
  fscanf(file, "%d", &res);
  return res;
}

int read_char_from_file(char filename[]){
  FILE *file;
 int f_char;
  file = fopen(filename, "rt");
  f_char = fgetc(file);
  return f_char;
}

long int disk_usage(){
  struct statvfs sbuf;
  if (statvfs("/", &sbuf) < 0){
    return -1;
  }
  return sbuf.f_bsize*sbuf.f_bavail;
}

int cpu_therm(){
  int input = 0;
  input = read_int_from_file("/sys/class/hwmon/hwmon0/temp2_input");
  input = input*0.001;
  return input;
}

int battery_percent(){
  long int full_bat;
  long int now_bat;
  full_bat = read_int_from_file("/sys/class/power_supply/BAT1/charge_full");
  now_bat = read_int_from_file("/sys/class/power_supply/BAT1/charge_now");
  return now_bat*100/full_bat;
}

int battery_status(){
  int b_stat = 0;
  const int ac_on = 67;
  b_stat = read_char_from_file("/sys/class/power_supply/BAT1/status");
  if (DEBUG) {
    fprintf(stderr, "BATTERY STATUS: %d\n\r", b_stat);
  }
  return (b_stat==ac_on);
}

char *now_time(char *buffer){
  time_t rawtime;
  struct tm * timeinfo;
  time(&rawtime);
  timeinfo = localtime ( &rawtime );
  strftime(buffer, 80, "%c", timeinfo);
  return buffer;
}

int main( int arg_count, char *argv[] ) {
  long int free_bytes = 0;
  float free_gb = 0;
  float cpu_t = 0;
  int battery = 0;
  char battery_stat_label[] = "off";
  char time_now[80];
  setlocale(LC_ALL, "");
  if (DEBUG) {
    fprintf(stderr, "Locale is: %s\n", setlocale(LC_ALL,NULL) );
  }
  while (1) {
    free_bytes = disk_usage();
    free_gb = ToGB(free_bytes);
    cpu_t = cpu_therm();
    battery = battery_percent();
    if (battery_status()) {
      strcpy(battery_stat_label, "on");
    } else {
      strcpy(battery_stat_label, "off");
    }
    now_time(time_now);
    printf("Free space: %.0fGB | CPU: %.2f°C | Battery: %d%% AC:%s | %s\n\r", free_gb, cpu_t, battery, battery_stat_label, time_now);
    sleep(1);
  }
  return 0;
}

