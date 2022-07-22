from setuptools import setup

with open("README", 'r') as f:
    long_description = f.read()

setup(
   name='prizm',
   version='1.0',
   description='A debug tool',
   license="MIT",
   long_description=long_description,
   author='Frank Bender',
   author_email='bende263@umn.edu',
   url="../build/lib/prizm.so",
   packages=['logger'],  #same as name
#    install_requires=[], #external packages as dependencies
#    scripts=[
#             'scripts/cool',
#             'scripts/skype',
#            ]
)