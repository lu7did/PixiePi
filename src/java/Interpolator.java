package ji3gab.dsp;

public class Interpolator {
  private int factor;		// interpolation factor
  private float coeff[];	// filter coefficients
  private int n_tap;		// number of taps(coefficients)
  private int in_idx;		// index at which new data will be inserted
  private float buf[];	// used as a circular buffer

  Interpolator(float a[], int n_tap, int factor) {
    in_idx = 0;
    this.n_tap = n_tap;
    coeff = a;
    this.factor = factor;

    int buf_size = (n_tap / factor) + 1;
    buf = new float[buf_size];
  }

  // interpolate:
  // takes an array of samples and compute
  // interpolated output. Filtering is performed after
  // interpolation to avoid aliasing.
  // It omits unnecessary multiply with inserted zeros.
  
  public void interpolate(float x[], int len, float out[]) {
    int m = 0;			// output index
    
    for (int k = 0; k < len; k++) {
      buf[in_idx] = x[k];
      for (int n = 0; n < factor; n++) {
	
	double y = 0.0;
	int j = in_idx;
	
	for (int i = n; i < n_tap; i+=factor) {
	  if (j < 0)
	    j += buf.length;
	  y = y + coeff[i] * buf[j--];
	}
	out[m++] = (float)(factor * y);
      }
      in_idx++;
      if (in_idx >= buf.length) {
	in_idx -= buf.length;
      }
    }
  }
}
