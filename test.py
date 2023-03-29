import subprocess
import random

# Define a function to check the format of a single line
def check_line(line):
    # Extract the integer values from the line
    line = line[4:-1]
    #print(line)
    values = line.split(',')
    for i in range(0,len(values)):
        if values[i] == '': continue
        values[i] = int(values[i].strip())

    return values

for o in range(10):
    # Run the bash command and capture the output
    nums = random.randint(1,9) * 10 
    process = subprocess.run(['bash', 'run.sh', str(nums)], capture_output=True, text=True)

    # Split the output into lines
    output_lines = process.stdout.strip().split('\n')
    
    # Check each line that starts with Li, Ei, or Gi
    for line in output_lines:
        if line.startswith('L:'):
            x = check_line(line)
            lineX = line
        elif line.startswith('E:'):
            y = check_line(line)
            liney = line
        elif line.startswith('G:'):
            z = check_line(line)
            linez = line
        else:
            # Ignore lines that don't start with Li, Ei, or Gi
            continue
    if len(set(y)) == 1 and max(x, default=-1) < y[0] < min(z, default=256):
        print("SUCCESS")
    elif (len(z) == 0) and max(x, default=-1) < y[0]:
        print("SUCCESS")
    else:
        print("NOT GOOD")
        print(lineX, x)
        print(liney, y)
        print(linez, z)
