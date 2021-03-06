//========================================================================
//
// Function.h
//
// Copyright 2001-2002 Glyph & Cog, LLC
//
//========================================================================

#ifndef FUNCTION_H
#define FUNCTION_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "gtypes.h"
#include "Object.h"

class Dict;
class Stream;
struct PSObject;
class PSStack;

//------------------------------------------------------------------------
// Function
//------------------------------------------------------------------------

#define funcMaxInputs  8
#define funcMaxOutputs 8

class Function {
public:

  Function();

  virtual ~Function();

  // Construct a function.  Returns NULL if unsuccessful.
  static Function *parse(Object *funcObj);

  // Initialize the entries common to all function types.
  GBool init(Dict *dict);

  virtual Function *copy() const = 0;

  // Return size of input and output tuples.
  int getInputSize() const { return m; }
  int getOutputSize() const { return n; }

  // Transform an input tuple into an output tuple.
  virtual void transform(const double *in, double *out) const = 0;

  virtual GBool isOk() const = 0;

protected:

  int m, n;			// size of input and output tuples
  double			// min and max values for function domain
    domain[funcMaxInputs][2];
  double			// min and max values for function range
    range[funcMaxOutputs][2];
  GBool hasRange;		// set if range is defined
};

//------------------------------------------------------------------------
// IdentityFunction
//------------------------------------------------------------------------

class IdentityFunction: public Function {
public:

  IdentityFunction();
  virtual ~IdentityFunction();
  virtual Function *copy() const { return new IdentityFunction(); }
  virtual void transform(const double *in, double *out) const;
  virtual GBool isOk() const { return gTrue; }

private:
};

//------------------------------------------------------------------------
// SampledFunction
//------------------------------------------------------------------------

class SampledFunction: public Function {
public:

  SampledFunction(Object *funcObj, Dict *dict);
  virtual ~SampledFunction();
  virtual Function *copy() const { return new SampledFunction(this); }
  virtual void transform(const double *in, double *out) const;
  virtual GBool isOk() const { return ok; }

private:

  SampledFunction(const SampledFunction *func);

  int				// number of samples for each domain element
    sampleSize[funcMaxInputs];
  double			// min and max values for domain encoder
    encode[funcMaxInputs][2];
  double			// min and max values for range decoder
    decode[funcMaxOutputs][2];
  double *samples;		// the samples
  GBool ok;
};

//------------------------------------------------------------------------
// ExponentialFunction
//------------------------------------------------------------------------

class ExponentialFunction: public Function {
public:

  ExponentialFunction(Object *funcObj, Dict *dict);
  virtual ~ExponentialFunction();
  virtual Function *copy() const { return new ExponentialFunction(this); }
  virtual void transform(const double *in, double *out) const;
  virtual GBool isOk() const { return ok; }

private:

  ExponentialFunction(const ExponentialFunction *func);

  double c0[funcMaxOutputs];
  double c1[funcMaxOutputs];
  double e;
  GBool ok;
};

//------------------------------------------------------------------------
// StitchingFunction
//------------------------------------------------------------------------

class StitchingFunction: public Function {
public:

  StitchingFunction(Object *funcObj, Dict *dict);
  virtual ~StitchingFunction();
  virtual Function *copy() const { return new StitchingFunction(this); }
  virtual void transform(const double *in, double *out) const;
  virtual GBool isOk() const { return ok; }

private:

  StitchingFunction(const StitchingFunction *func);

  int k;
  Function **funcs;
  double *bounds;
  double *encode;
  GBool ok;
};

//------------------------------------------------------------------------
// PostScriptFunction
//------------------------------------------------------------------------

class PostScriptFunction: public Function {
public:

  PostScriptFunction(Object *funcObj, Dict *dict);
  virtual ~PostScriptFunction();
  virtual Function *copy() const { return new PostScriptFunction(this); }
  virtual void transform(const double *in, double *out) const;
  virtual GBool isOk() const { return ok; }

private:

  PostScriptFunction(const PostScriptFunction *func);
  GBool parseCode(Stream *str, int *codePtr);
  GString *getToken(Stream *str);
  void resizeCode(int newSize);
  void exec(PSStack *stack, int codePtr) const;

  PSObject *code;
  int codeSize;
  GBool ok;
};

#endif
