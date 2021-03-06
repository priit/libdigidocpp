/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.6
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package ee.ria.libdigidocpp;

public class StringVector {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected StringVector(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(StringVector obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        digidocJNI.delete_StringVector(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public StringVector() {
    this(digidocJNI.new_StringVector__SWIG_0(), true);
  }

  public StringVector(long n) {
    this(digidocJNI.new_StringVector__SWIG_1(n), true);
  }

  public long size() {
    return digidocJNI.StringVector_size(swigCPtr, this);
  }

  public long capacity() {
    return digidocJNI.StringVector_capacity(swigCPtr, this);
  }

  public void reserve(long n) {
    digidocJNI.StringVector_reserve(swigCPtr, this, n);
  }

  public boolean isEmpty() {
    return digidocJNI.StringVector_isEmpty(swigCPtr, this);
  }

  public void clear() {
    digidocJNI.StringVector_clear(swigCPtr, this);
  }

  public void add(String x) {
    digidocJNI.StringVector_add(swigCPtr, this, x);
  }

  public String get(int i) {
    return digidocJNI.StringVector_get(swigCPtr, this, i);
  }

  public void set(int i, String val) {
    digidocJNI.StringVector_set(swigCPtr, this, i, val);
  }

}
