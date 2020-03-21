ackage ji3gab.dsp;

public class Decimator {
  private int factor;		// decimation factor
  private float coeff[];	// filter coefficients
  private int n_tap;		// number of taps(coefficients)
  private int in_idx;		// index at which new data will be inserted
  private float buf[];	// used as a circular buffer

  Decimator(float a[], int n_tap, int factor) {
    in_idx = 0;
    this.n_tap = n_tap;
    coeff = a;
    buf = new float[n_tap];
    this.factor = factor;
  }

  // decimate:
  // take an array of samples and compute
  // decimated output. Filtering is performed before
  // decimation to avoid aliasing.
  // It omits unnecessary computation for outputs which will be
  // thrown away later.
  
  public void decimate(float x[], int len, float out[]) {
    int m = 0;			// output index

    for (int k = 0; k < len; ) {
      
      for (int n = 0; n < factor; n++) {
	buf[in_idx++] = x[k++];
	if (in_idx >= n_tap) {
	  in_idx -= n_tap;
	}
      }
      
      int j = in_idx - 1;
      if (j < 0)
	j = n_tap - 1;
      double y = 0.0;    
      for (int i = 0; i < n_tap; ++i) {
	if (j < 0)
	  j += n_tap;
	y = y + coeff[i] * buf[j--];
      }
      out[m++] = (float)y;
    }
  }
}
