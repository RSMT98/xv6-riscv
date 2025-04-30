#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define SECONDS_IN_MINUTE 60
#define MINUTES_IN_HOUR 60
#define SECONDS_IN_HOUR (MINUTES_IN_HOUR * SECONDS_IN_MINUTE)
#define HOURS_IN_DAY 24
#define SECONDS_IN_DAY (HOURS_IN_DAY * SECONDS_IN_HOUR)
#define DAYS_IN_YEAR 365
#define DAYS_IN_LEAP_YEAR 366
#define START_YEAR 1970

struct date
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    uint64 nanosecond;
};

static int is_leap(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static struct date calc_date_from_ns_time(uint64 ns_time)
{
    struct date dt;
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint64 secs = ns_time / 1000000000;
    dt.nanosecond = ns_time % 1000000000;

    uint64 secs_today = secs % SECONDS_IN_DAY;
    dt.hour = secs_today / SECONDS_IN_HOUR;
    dt.minute = (secs_today % SECONDS_IN_HOUR) / SECONDS_IN_MINUTE;
    dt.second = secs_today % SECONDS_IN_MINUTE;

    uint64 total_days = secs / SECONDS_IN_DAY;
    int year = START_YEAR;
    int days_in_year;
    while (total_days >= (days_in_year = is_leap(year) ? DAYS_IN_LEAP_YEAR : DAYS_IN_YEAR))
    {
        total_days -= days_in_year;
        ++year;
    }
    dt.year = year;

    int month = 0;
    days_in_month[1] = is_leap(year) ? 29 : 28;
    while (total_days >= days_in_month[month])
    {
        total_days -= days_in_month[month];
        ++month;
    }
    dt.month = month + 1;

    dt.day = total_days + 1;
    return dt;
}

int main(int argc, char *argv[])
{
    uint64 ns_time = get_time();
    struct date dt = calc_date_from_ns_time(ns_time);
    printf("%d-%s%d-%s%d %s%d:%s%d:%s%d.%s%ld\n",
           dt.year,
           dt.month < 10 ? "0" : "", dt.month,
           dt.day < 10 ? "0" : "", dt.day,
           dt.hour < 10 ? "0" : "", dt.hour,
           dt.minute < 10 ? "0" : "", dt.minute,
           dt.second < 10 ? "0" : "", dt.second,
           dt.nanosecond < 100000000 ? (dt.nanosecond < 10000000 ? (dt.nanosecond < 1000000 ? (dt.nanosecond < 100000 ? (dt.nanosecond < 10000 ? (dt.nanosecond < 1000 ? (dt.nanosecond < 100 ? (dt.nanosecond < 10 ? "00000000" : "0000000") : "000000") : "00000") : "0000") : "000") : "00") : "0") : "", dt.nanosecond);

    exit(0);
}
