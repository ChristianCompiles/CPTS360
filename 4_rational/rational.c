#include <stdio.h> // for printf()
#include <stdlib.h> // for exit()
#include "rational.h"
 
static Rational rtnl_simplify(Rational rtnl)
{
    int a = abs(rtnl.num);
    int b = abs(rtnl.denom);
    int tmp = 0;
 
    while(b != 0)
    {
        tmp = b;
        b = a % b;
        a = tmp;
    }

    rtnl.num /= a;
    rtnl.denom /= a;

    if ((rtnl.num < 0 && rtnl.denom <0) || rtnl.denom <0) // if num and denom are negative, or just denom is negative, then change sign of top and bottom
    {
        rtnl.num *= -1;
        rtnl.denom *= -1;
    }
    return rtnl;
}

Rational rtnl_add(Rational rtnl0, Rational rtnl1)
{
       Rational summed = {0,1};
       summed.num = (rtnl0.num*rtnl1.denom) + (rtnl0.denom*rtnl1.num);
       summed.denom = rtnl0.denom * rtnl1.denom;

       return rtnl_simplify(summed);
}

Rational rtnl_sub(Rational rtnl0, Rational rtnl1)
{
       // negative numerator of second rational
       rtnl1.num *= -1;
       return rtnl_add(rtnl0, rtnl1);
}

Rational rtnl_mul(Rational rtnl0, Rational rtnl1)
{
       Rational multed = {1,1};
       multed.num = rtnl0.num * rtnl1.num;
       multed.denom = rtnl0.denom * rtnl1.denom;

       return rtnl_simplify(multed);
}

Rational rtnl_div(Rational rtnl0, Rational rtnl1)
{
       // reciprocate rtnl1
       int tmp = rtnl1.num;
       rtnl1.num = rtnl1.denom;
       rtnl1.denom = tmp;

       return rtnl_mul(rtnl0, rtnl1);
}

Rational rtnl_init(int num, int denom)
{
       // can do a check for zero here
       Rational inited = {num, denom};
       return rtnl_simplify(inited);
}

Rational rtnl_ipow(Rational rtnl, int ipow)
{
       Rational rtnl_ones = {1,1};

       if(ipow == 0) // pow 0 equals 1
              return rtnl_ones;
       
       if(ipow == 1)
              return rtnl;

       // now powered will be used to accumulate multiplies
       Rational powered = {1,1};
       powered.num = rtnl.num;
       powered.denom = rtnl.denom;

       for(int i = 0; i < ipow-1; i++)
       {      
              powered = rtnl_mul(powered, rtnl);
       }            
       
       if(ipow < 0)
              return rtnl_div(rtnl_ones, powered);
       else
              return powered;
}

char *rtnl_asStr(Rational rtnl, char buf[RTNL_BUF_SIZE])
{
       sprintf(buf, "(%d/%d)", rtnl.num, rtnl.denom);
       return buf;
} 