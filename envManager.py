from os.path import isfile
Import("env")

assert isfile(".env")

try:
    with open(".env", "r") as f:
        lines = f.readlines()
        envs = []
        for line in lines:
            if not line.strip() or line.strip().startswith("#"):
                continue
            key, value = line.strip().split('=')
            # Check if the value is an integer
            if value.isdigit():
                envs.append("-D{}={}".format(key, value))
            if "," in value:
                l = value.split(",")
                r = []
                for i in l:
                    r += [ f'"{i}"' ]
                envs.append("-D{}='{}'".format(key, ",".join(r)))
                envs.append("-D{}_LEN={}".format(key, len(l)))
            else:
                envs.append('-D{}="{}"'.format(key, value))
        env.Append(BUILD_FLAGS=envs)
except IOError:
    print("File .env not accessible")
