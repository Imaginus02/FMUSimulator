import csv
import matplotlib.pyplot as plt
import os

file_path = os.path.join(os.path.dirname(__file__), 'out.csv')
with open(file_path, 'r') as file:
	reader = csv.reader(file)
	header = next(reader)
	data = list(reader)

# Extract the column names
columns = {name: [] for name in header}

# Populate the columns with data
for row in data:
	for name, value in zip(header, row):
		columns[name].append(float(value))

# Extract the first column as the x-axis
x_values = columns[header[0]]

# Create subplots for each variable against the first column, excluding the first column itself
fig, axes = plt.subplots(len(header) - 1, 1, sharex=True, figsize=(10, 8))

# Plot each variable against the first column, excluding the first column itself
for i, name in enumerate(header[1:]):
	axes[i].plot(x_values, columns[name])
	axes[i].set_ylabel(name)
	axes[i].set_title(f'{name} vs {header[0]}')
	axes[i].grid(True)

axes[-1].set_xlabel(header[0])

# Adjust layout and save the plot
plt.tight_layout()
plt.savefig(os.path.join(os.path.dirname(__file__), 'out.png'))
