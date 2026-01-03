import numpy as np
import matplotlib.pyplot as plt

def exponential_lowpass_filter(input_signal, alpha, initial_output=0):
    """
    Apply exponential low-pass filter to input signal.

    Parameters:
    - input_signal: array of input values
    - alpha: filter coefficient (0 < alpha <= 1)
    - initial_output: initial filter output value

    Returns:
    - filtered output signal
    """
    output = np.zeros_like(input_signal)
    output[0] = alpha * input_signal[0] + (1 - alpha) * initial_output

    for n in range(1, len(input_signal)):
        output[n] = alpha * input_signal[n] + (1 - alpha) * output[n-1]

    return output

# Create a step input signal
samples = 50
step_input = np.zeros(samples)
step_input[10: ] = 1.0  # Step change at sample 10

# Test different alpha values
alpha_values = [0.1, 0.2, 0.3, 0.46, 0.5, 0.8]

# Create subplots
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10))

# Plot 1: Step responses for different alpha values
ax1.plot(step_input, 'k--', linewidth=2, label='Input (Step)', alpha=0.7)

for alpha in alpha_values:
    output = exponential_lowpass_filter(step_input, alpha)
    ax1.plot(output, marker='o', markersize=4, label=f'α = {alpha}')

ax1.axhline(y=0.95, color='r', linestyle=':', alpha=0.5, label='95% level')
ax1.axvline(x=10, color='gray', linestyle=':', alpha=0.3)
ax1.grid(True, alpha=0.3)
ax1.set_xlabel('Sample Number', fontsize=12)
ax1.set_ylabel('Output Value', fontsize=12)
ax1.set_title('Exponential Low-Pass Filter: Step Response for Different α Values', fontsize=14)
ax1.legend(loc='right', fontsize=10)
ax1.set_xlim(0, samples-1)

# Plot 2: Settling time analysis
settling_times_95 = []
settling_times_99 = []

for alpha in np.linspace(0.05, 1.0, 50):
    output = exponential_lowpass_filter(step_input, alpha)

    # Find 95% settling time (from step point)
    step_index = 10
    target_95 = 0.95
    target_99 = 0.99

    try:
        settling_95 = np.where(output[step_index:] >= target_95)[0][0]
        settling_times_95.append(settling_95)
    except:
        settling_times_95.append(np.nan)

    try:
        settling_99 = np.where(output[step_index:] >= target_99)[0][0]
        settling_times_99.append(settling_99)
    except:
        settling_times_99.append(np.nan)

alpha_range = np.linspace(0.05, 1.0, 50)
ax2.plot(alpha_range, settling_times_95, 'b-', linewidth=2, label='95% Settling Time')
ax2.plot(alpha_range, settling_times_99, 'r-', linewidth=2, label='99% Settling Time')
ax2.axhline(y=10, color='g', linestyle='--', linewidth=2, label='Target:  10 samples')
ax2.grid(True, alpha=0.3)
ax2.set_xlabel('Alpha (α)', fontsize=12)
ax2.set_ylabel('Settling Time (samples)', fontsize=12)
ax2.set_title('Settling Time vs Alpha Value', fontsize=14)
ax2.legend(fontsize=10)
ax2.set_xlim(0.05, 1.0)
ax2.set_ylim(0, 30)

# Add annotations for recommended alpha values
ax2.annotate('α ≈ 0.3\n(10 sample, 95%)',
             xy=(0.3, 10), xytext=(0.35, 15),
             arrowprops=dict(arrowstyle='->', color='blue', lw=1.5),
             fontsize=10, color='blue', fontweight='bold')

ax2.annotate('α ≈ 0.46\n(10 sample, 99%)',
             xy=(0.46, 10), xytext=(0.5, 5),
             arrowprops=dict(arrowstyle='->', color='red', lw=1.5),
             fontsize=10, color='red', fontweight='bold')

plt.tight_layout()
plt.savefig('lowpass_filter_analysis.png', dpi=300, bbox_inches='tight')
plt.show()

# Print exact settling times for key alpha values
print("\n" + "="*60)
print("SETTLING TIME ANALYSIS")
print("="*60)
print(f"{'Alpha':<10} {'95% Time':<15} {'99% Time':<15} {'Time Constant (τ)':<20}")
print("-"*60)

for alpha in [0.1, 0.2, 0.3, 0.4, 0.46, 0.5]:
    output = exponential_lowpass_filter(step_input, alpha)
    step_index = 10

    try:
        time_95 = np.where(output[step_index:] >= 0.95)[0][0]
    except:
        time_95 = "N/A"

    try:
        time_99 = np.where(output[step_index:] >= 0.99)[0][0]
    except:
        time_99 = "N/A"

    tau = 1/alpha

    print(f"{alpha:<10. 2f} {str(time_95):<15} {str(time_99):<15} {tau:<20.2f}")
