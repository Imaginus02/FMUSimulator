import matplotlib.pyplot as plt

# Read the data from the file
with open('/mnt/d/Seafile/ProjetIndus_Reynaud/test_simulator/tests/out.txt', 'r') as file:
	lines = file.readlines()

# Initialize lists to store time, h, and v values
time_values = []
h_values = []
v_values = []

# Extract time, h, and v values from the file
for line in lines:
	if line.startswith('time:'):
		time_values.append([float(value) for value in line.split(' ')[1:-1]])
	elif line.startswith('h:'):
		h_values.append([float(value) for value in line.split(' ')[1:-1]])
	elif line.startswith('v:'):
		v_values.append([float(value) for value in line.split(' ')[1:-1]])

# Create subplots
fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True)

# Plot h vs time
for t, h in zip(time_values, h_values):
	ax1.plot(t, h)
ax1.set_ylabel('h')
ax1.set_title('h vs Time')
ax1.grid(True)

# Plot v vs time
for t, v in zip(time_values, v_values):
	ax2.plot(t, v)
ax2.set_xlabel('Time')
ax2.set_ylabel('v')
ax2.set_title('v vs Time')
ax2.grid(True)

# Save the plot
plt.savefig('/mnt/d/Seafile/ProjetIndus_Reynaud/test_simulator/tests/out.png')