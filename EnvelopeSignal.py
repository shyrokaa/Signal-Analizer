import numpy as np
from scipy.signal import hilbert

signal = np.loadtxt("waveData.txt", comments="#", delimiter="\n", unpack=False)

analytic_signal = hilbert(signal)
amplitude_envelope = np.abs(analytic_signal)

np.savetxt("waveEnv.txt", amplitude_envelope, fmt="%2.0f")

print(amplitude_envelope)