from distutils.core import setup
import py2exe

setup(windows=[{"script" : "quark.py"}],
      options={"py2exe" : {"includes" : ["sip"]}})
