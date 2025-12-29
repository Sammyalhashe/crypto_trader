import re
import sys

def check_naming_conventions(file_path):
    errors = []
    with open(file_path, 'r') as f:
        lines = f.readlines()

    for i, line in enumerate(lines):
        # Check for member variables
        if 'd_' in line:
            match = re.search(r'\s+(d_[a-zA-Z0-9_]+);', line)
            if match:
                variable_name = match.group(1)
                if not re.match(r'^d_[a-z][a-zA-Z0-9]*(_p)?$', variable_name):
                    errors.append(f"{file_path}:{i+1}: Member variable '{variable_name}' does not follow the naming convention.")

        # Check for local variables
        match = re.search(r'\s+([a-z][a-zA-Z0-9]*) = .*;', line)
        if match:
            variable_name = match.group(1)
            if 'd_' not in variable_name and not re.match(r'^[a-z][a-zA-Z0-9]*$', variable_name):
                errors.append(f"{file_path}:{i+1}: Variable '{variable_name}' does not follow the camelCase naming convention.")

    return errors

if __name__ == '__main__':
    all_errors = []
    for file_path in sys.argv[1:]:
        if file_path.endswith('.h') or file_path.endswith('.cpp'):
            all_errors.extend(check_naming_conventions(file_path))

    if all_errors:
        for error in all_errors:
            print(error)
        sys.exit(1)
    
    print("Naming convention check passed.")
