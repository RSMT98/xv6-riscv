#ifndef PS_H
#define PS_H

#define PS_MAJOR 2
void ps_init(void);

enum ps_minor
{
  NULL,
  ZERO,
  URANDOM,
  NULLSTAT
};

#endif
