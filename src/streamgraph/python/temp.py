import numpy as np 
import matplotlib.pyplot as plt
from scipy.signal import freqz

L              = 3               
TAPS_PER_PHASE = 16             
NUM_TAPS       = (L * TAPS_PER_PHASE)

def coefs():
    n = np.arange(0,NUM_TAPS)
    h = np.zeros(NUM_TAPS)
    fc = 0.5 / L     # normalized cutoff (0..0.5 w.r.t input rate)
    m = n - (NUM_TAPS - 1) / 2.0
    sinc = np.sinc(2 * fc * m)
    w = 0.54 - 0.46 * np.cos(2.0 * np.pi * n / (NUM_TAPS - 1)) # Hamming
    h = sinc * w
   
    
    # Optional: normalize DC gain to 1 (nice for unity passband)
    sum = 0.0
    sum = np.sum(h)
    h = h / sum
    return(h)

h = coefs()

print(np.array2string(h, separator=', '))
print(len(h))

exit(0)

plt.figure(figsize=(8, 4))
plt.stem(h, basefmt=" ")
plt.title(f"Interpolation prototype FIR coefficients (L={L})")
plt.xlabel("Tap index")
plt.ylabel("Amplitude")
plt.grid(True)

# --- Frequency response ---
w, H = freqz(h, worN=1024)
plt.figure(figsize=(8, 4))
plt.plot(w / np.pi, 20 * np.log10(np.abs(H) + 1e-10))
plt.title("Frequency Response (magnitude)")
plt.xlabel("Normalized frequency (×π rad/sample)")
plt.ylabel("Magnitude (dB)")
plt.ylim(-120, 5)
plt.grid(True)

plt.show()
