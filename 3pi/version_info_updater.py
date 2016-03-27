from time import strftime as time
f = open('../version_info.hpp', 'r')
last = f.read()
f.close()
build_str = '"build '
build = last.find(build_str, 32) + len(build_str)
build = int(last[build:last.find(':', build)]) + 1
build_info = '{}{}: {}'.format(build_str[1:], build, time('%H:%M:%S %d.%m.%Y'))
f = open('../version_info.hpp', 'w')
f.write('const char build_info[] PROGMEM = "{}";\n'.format(build_info));
f.close()
print build_info