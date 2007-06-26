/*
 
Copyright (C) 2007 Lucas K. Wagner

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 
*/

#include "Spline_fitter.h"

void Spline_fitter::raw_input(ifstream & input)
{

  int m;
  doublevar dummy;
  input >> m >> spacing >> threshold;
  coeff.Resize(m,4);

  for(int k=0; k<m; k++) {
    input >> dummy;
    for(int i=0; i<4; i++) {
      input >> dummy;
      //if(fabs(dummy-coeff(s,k,i)) > 1e-4) {
      //  cout << "big diff in spline "<< s
      //       << "  k= " << k << " i= " << i << endl;
      //  cout << "read " << dummy << "  calculated " << coeff(s,k,i)
      //       <<endl;
      //}
      coeff(k,i)=dummy;
    }
  }
}



//----------------------------------------------------------------------


int Spline_fitter::readspline(vector <string> & words) {

  int n=(words.size()-1)/2;
  if(n < 4) {
    error("Not enough points to fit a spline.");
  }

  coeff.Resize( n, 4);

  //cout << "fitting spline " << s << endl;
  Array1 <doublevar> x(n), y(n);
  //cout << "number of points I expect: " << 2*n+1 << " number found " << splinefits[s].size()
    //<< endl;

  int counter=1;
  for(int i=0; i < n; i++) {
    x(i)=atof(words[counter++].c_str());
    y(i)=atof(words[counter++].c_str());
  }

  spacing=x(1)-x(0);
  //cout << "spacing " << spacing << endl;
  //Check that the points are monotonically increasing with equal spacing
  //This is mostly needed for the evaluation routine.
  for(int i=1; i< n; i++) {
    if(fabs(x(i)-x(i-1)-spacing) > 1e-10) {
      error("I need equal spacing for all the spline points.");
    }
  }

  threshold=x(n-1);

  //--Estimate the derivatives on the boundaries
  doublevar yp1=(y(1)-y(0))/spacing;
  doublevar ypn=(y(n-1) - y(n-2) ) /spacing;
  splinefit(x, y, yp1, ypn);

  return 1;
}

//----------------------------------------------------------------------


doublevar Spline_fitter::findCutoff() {
  const doublevar step=0.5;
  doublevar cutoff_threshold=1e-10;

  int n=coeff.GetDim(0);
  doublevar max=0;
  for(int i=0; i< n; i++) {
    if(fabs(coeff(i,0)) > max)
      max=fabs(coeff(i,0));
  }

  cutoff_threshold*=max;


  for(doublevar x=threshold-step; x >=0; x-=step) {
    int interval=getInterval(x);
    doublevar height=x-interval*spacing;
    doublevar test= coeff(interval,0)+height*(coeff(interval,1)
                          +height*(coeff(interval,2)
                          +height*coeff(interval,3)));
    if(fabs(test) > cutoff_threshold) {
      //cout << "cutoff " << x+step << endl;
      return x+step;
    }
  }
  return 0;
}

//----------------------------------------------------------------------


void Spline_fitter::splinefit(Array1 <doublevar>& x, Array1 <doublevar>& y,
                             double yp1, double ypn)
{
  //following stolen from Numerical Recipes, more or less

  assert(x.GetDim(0)==y.GetDim(0));

  int n=x.GetDim(0);
  coeff.Resize(n,4);

  spacing=x(1)-x(0);
  threshold=x(n-1)+spacing;

  Array1 <doublevar> y2(n), u(n);
  doublevar sig, p, qn, un, hi;

  if(yp1 > .99e30) {
    y2(0)=0;
    u(0)=0;
  }
  else {
    y2(0)=-.5;
    u(0)=(3./(x(1)-x(0)))*((y(1)-y(0))/(x(1)-x(0))-yp1);
  }

  for(int i=1; i<n-1; i++)
  {
    sig=(x(i)-x(i-1))/(x(i+1)-x(i-1));
    p=sig*y2(i-1)+2.;
    y2(i)=(sig-1.)/p;
    u(i)=(6.*((y(i+1)-y(i))/(x(i+1)-x(i))
              -(y(i)-y(i-1))/(x(i)-x(i-1)))
          /(x(i+1)-x(i-1))-sig*u(i-1))/p;
  }

  if(ypn>.99e30)
  {
    qn=0;
    un=0;
  }
  else
  {
    qn=.5;
    un=(3./(x(n-1)-x(n-2)))*(ypn-(y(n-1)-y(n-2))/(x(n-1)-x(n-2)));
  }

  y2(n-1)=(un-qn*u(n-2))/(qn*y2(n-2)+1.);

  for(int k=n-2; k>=0; k--)
  {
    y2(k)=y2(k)*y2(k+1)+u(k);
  }

  for(int i=0; i<n-1; i++)
  {
    coeff(i,0)=y(i);
    hi=x(i+1)-x(i);
    coeff(i,1)=(y(i+1)-y(i))/hi - hi*(2.*y2(i)+y2(i+1))/6.;
    coeff(i,2)=y2(i)/2.;
    coeff(i,3)=(y2(i+1)-y2(i))/(6.*hi);
  }
  coeff(n-1, 0)=y(n-1);
  coeff(n-1, 1)=0;
  coeff(n-1, 2)=0;
  coeff(n-1, 3)=0;

  //cout << "spline fit " << endl;
  
  //  for(int i=0; i< n; i++) {
  ///    cout << i*(x(1)-x(0)) << "   ";
  //    for(int j=0; j< 4; j++) {
  //        cout << coeff(i,j) << "    ";
  //    }
  //    cout << endl;
  //  }
  

}

//######################################################################



doublevar Spline_fitter_nonuniform::findCutoff() {
  const doublevar step=0.5;
  const doublevar cutoff_threshold=1e-8;

  for(doublevar x=threshold-step; x >=0; x-=step) {
    int interval=getInterval(x);
    doublevar height=x-pos(interval);
    doublevar test= coeff(interval,0)+height*(coeff(interval,1)
                          +height*(coeff(interval,2)
                          +height*coeff(interval,3)));
    if(fabs(test) > cutoff_threshold) {
      //cout << "cutoff " << x+step << endl;
      return x+step;
    }
  }
  return 0;
}

//----------------------------------------------------------------------


void Spline_fitter_nonuniform::splinefit(Array1 <doublevar>& x, 
                                         Array1 <doublevar>& y,
                                         double yp1, double ypn)
{
  //following stolen from Numerical Recipes, more or less

  assert(x.GetDim(0)==y.GetDim(0));

  int n=x.GetDim(0);
  coeff.Resize(n,4);
  pos=x;


  threshold=x(n-1);

  Array1 <doublevar> y2(n), u(n);
  doublevar sig, p, qn, un, hi;

  if(yp1 > .99e30) {
    y2(0)=0;
    u(0)=0;
  }
  else {
    y2(0)=-.5;
    u(0)=(3./(x(1)-x(0)))*((y(1)-y(0))/(x(1)-x(0))-yp1);
  }

  for(int i=1; i<n-1; i++)
  {
    sig=(x(i)-x(i-1))/(x(i+1)-x(i-1));
    p=sig*y2(i-1)+2.;
    y2(i)=(sig-1.)/p;
    u(i)=(6.*((y(i+1)-y(i))/(x(i+1)-x(i))
              -(y(i)-y(i-1))/(x(i)-x(i-1)))
          /(x(i+1)-x(i-1))-sig*u(i-1))/p;
  }

  if(ypn>.99e30)
  {
    qn=0;
    un=0;
  }
  else
  {
    qn=.5;
    un=(3./(x(n-1)-x(n-2)))*(ypn-(y(n-1)-y(n-2))/(x(n-1)-x(n-2)));
  }

  y2(n-1)=(un-qn*u(n-2))/(qn*y2(n-2)+1.);

  for(int k=n-2; k>=0; k--)
  {
    y2(k)=y2(k)*y2(k+1)+u(k);
  }

  for(int i=0; i<n-1; i++)
  {
    coeff(i,0)=y(i);
    hi=x(i+1)-x(i);
    coeff(i,1)=(y(i+1)-y(i))/hi - hi*(2.*y2(i)+y2(i+1))/6.;
    coeff(i,2)=y2(i)/2.;
    coeff(i,3)=(y2(i+1)-y2(i))/(6.*hi);
  }
  coeff(n-1, 0)=y(n-1);
  coeff(n-1, 1)=0;
  coeff(n-1, 2)=0;
  coeff(n-1, 3)=0;

}

//----------------------------------------------------------------------

int Spline_fitter_nonuniform::getInterval(doublevar r) {

  int npts=pos.GetDim(0);
  int upper=npts;
  int lower=0;
  int guess=int(npts/2);

  //cout << "r= " << r << endl;

  while(true) {
    if(r >= pos(guess)) {
      //cout << "greater " << pos(guess) << endl;
      if(r < pos(guess+1) ) {
        //cout << "found it " << pos(guess+1) << endl;
        return guess;
      }
      else { 
        lower=guess;
        guess=(lower+upper)/2;
        //cout << "didn't find it: new lower " << lower 
        //     << " new guess " << guess << endl;
      }
    }
    if(r < pos(guess) ) {
      upper=guess;
      guess=(lower+upper)/2;
      //cout << "less than " << pos(guess) << endl;
      //cout << "new upper " << upper 
      //     << " new guess " << guess << endl;
    }  
  } 
  
}
