from setuptools import Extension, setup  # type: ignore
from wheel.bdist_wheel import bdist_wheel  # type: ignore


class bdist_wheel_abi3(bdist_wheel):
    def get_tag(self):
        python, abi, plat = super().get_tag()

        if python.startswith("cp"):
            # on CPython, our wheels are abi3 and compatible back to 3.6
            return "cp36", "abi3", plat

        return python, abi, plat


lib = Extension(
    "batamiaou.cbatamiaou",
    language="c",
    sources=["extension/batamiaoumodule.c"],
    define_macros=[
        ("Py_LIMITED_API", "0x03060000")
    ],  # macro to use Python Limited API (here >=cp36)],
    py_limited_api=True,
)

# other parameters are defined in pyproject.toml
setup(
    packages=["batamiaou"],
    include_package_data=True,
    ext_modules=[lib],
    cmdclass={"bdist_wheel": bdist_wheel_abi3},
)
