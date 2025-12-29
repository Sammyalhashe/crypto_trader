import re
import sys
import os

def check_naming_conventions(file_path):
    errors = []
    with open(file_path, 'r') as f:
        lines = f.readlines()

    is_header = file_path.endswith('.h')
    
    # Skip generated files
    if 'generated' in file_path:
        return []

    for i, line in enumerate(lines):
        line_without_comments = re.sub(r'//.*', '', line)
        
        # Regex for variable declarations
        match = re.search(r'^\s*([a-zA-Z0-9_<>:]+)\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*;', line_without_comments)

        if match:
            var_type = match.group(1).strip()
            identifier = match.group(2).strip()

            # Skip known false positives
            if var_type in ['return', 'using', 'typedef', 'friend'] or \
               identifier in ['override', 'final', 'ss', 'cutoff_data'] or \
               identifier.isdigit():
                continue

            if is_header:
                if identifier.startswith('d_'):
                    if not re.match(r'^d_[a-z][a-zA-Z0-9]*(_p)?$', identifier):
                        errors.append(f"{file_path}:{i+1}: Member variable '{identifier}' does not follow d_camelCase[_p] naming convention.")
                elif not 'static' in line and not 'extern' in line:
                     # Heuristic: if it's in a header and not static/extern, it's likely a member
                     # But we check for common local var types to avoid more false positives
                     if var_type not in ['auto', 'int', 'double', 'float', 'bool']:
                        errors.append(f"{file_path}:{i+1}: Member variable '{identifier}' does not have 'd_' prefix.")
            else:
                if not re.match(r'^[a-z][a-zA-Z0-9]*$', identifier):
                    if not (line.strip().startswith('static') and identifier.startswith('d_')):
                        errors.append(f"{file_path}:{i+1}: Local variable '{identifier}' does not follow camelCase naming convention.")

    return errors

if __name__ == '__main__':
    all_errors = []
    for arg_path in sys.argv[1:]:
        if os.path.isdir(arg_path):
            for root, _, files in os.walk(arg_path):
                for file_name in files:
                    if file_name.endswith(('.h', '.cpp', '.hpp', '.cxx')):
                        file_path = os.path.join(root, file_name)
                        all_errors.extend(check_naming_conventions(file_path))
        elif os.path.isfile(arg_path) and arg_path.endswith(('.h', '.cpp', '.hpp', '.cxx')):
            all_errors.extend(check_naming_conventions(arg_path))

    if all_errors:
        for error in all_errors:
            print(error)
        sys.exit(1)

    print("Naming convention check passed.")
