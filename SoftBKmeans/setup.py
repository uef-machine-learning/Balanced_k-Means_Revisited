#!/usr/bin/env python
import numpy

import setuptools
from setuptools import setup, Extension

__version__ = "0.1"

#For debug:
# cargs = ['-O1', '-g', '-std=c++11', '-fopenmp', '-fpermissive',  '-Wall', '-D_PYTHON_LIB']
cargs = ['-fpermissive']

with open('../README.md', 'r', encoding='utf-8') as f:
    long_description = f.read()

module1 = Extension('balkmeans', sources=['py_interf.cpp'], include_dirs=['.'], extra_compile_args=cargs)
                        
ext_modules = [module1]
                      
setup(
    name='balkmeans',
    version='1.0',
    setup_requires=['wheel'],
    requires=['rapidfuzz'],
    python_requires='>=3',
    provides=['balkmeans'],
    description='Balanced k-Means',
    long_description=long_description,
    long_description_content_type='text/markdown',
    classifiers=[
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
    ],
    ext_modules=[module1]
)

