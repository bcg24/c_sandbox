import pandas
import matplotlib.pyplot as plt

# Read the CSV
data = pandas.read_csv('results.csv')

# Plot each temperature column vs time
for col in data.columns:
    if col.startswith('T'):  # Only plot T0, T1, T2, etc.
        plt.plot(data['time'], data[col], label=col)

plt.xlabel('Time (s)')
plt.ylabel('Temperature (K)')
plt.title('1D Transient Conduction in Copper Rod')
plt.legend()
plt.grid(True)
plt.savefig('temperature_plot.png')
plt.show()