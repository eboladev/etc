python setup.py py2exe
copy *.png dist
mkdir quark
copy dist\* quark
@echo ***
@echo And now... Make a ZIP of quark
