import sys
from Cheetah.Template import Template

output = sys.argv[2]
args = {}
for arg in sys.argv[3:]:
    key, value = arg.split('=')
    args[key] = value
t = Template(file=sys.argv[1], searchList=[args])
with open(output, 'w') as f:
    f.write(str(t))
