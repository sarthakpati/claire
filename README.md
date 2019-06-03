# CLAIRE

* Are you looking for **examples**? Check the [doc/examples](https://github.com/andreasmang/claire/tree/master/doc/examples) folder.
* Are in interested in **how CLAIRE works**? Check the [documentation](#clairedoc).
* Are you interested in **what CLAIRE is**? Read the [about](#claireabout) section.

## About <a name="claireabout"></a>

**CLAIRE** stands for *Constrained Large Deformation Diffeomorphic Image Registration*. It is a C/C++ software package for velocity-based diffeomorphic image registration in three dimensions. Its performance is optimized for multi-core systems. It uses MPI for data parallelism, and has been demonstrated to scale on several supercomputing platforms. CLAIRE can be executed on large-scale state-of-the-art computing systems as well as on local compute systems with limited resources.

<p align="center">
<img src="doc/figs/claire4brains.jpg" alt="CLAIRE4Brains"  width="800"/>
</p>

If there are any issues, you have questions, you would like to give us feedback or you have feature requests, do not hesitate to send an email to <andreas@math.uh.edu>.

If you plan on using CLAIRE in your research please cite the following manuscript: A. Mang, A. Gholami, C. Davatzikos & G. Biros. *CLAIRE: A distributed-memory solver for constrained large deformation diffeomorphic image registration*. arXiv:1808.0448. 2018.

## Documentation <a name="clairedoc"></a>
* [Quick Installation Guide](doc/README-INSTALL-QUICK.md)
* [Detailed Installation Guide](doc/README-INSTALL.md)
* [Examples](doc/README-RUNME.md)
* [News](doc/README-NEWS.md)
* [Publications](doc/README-REFERENCES.md)

The links above point to individual markdown files. These files can be found in the [doc](https://github.com/andreasmang/claire/tree/master/doc) subfolder. Basic examples for how to execute CLAIRE can be found in the [doc/examples](https://github.com/andreasmang/claire/tree/master/doc/examples) folder.

## Runtime Requirements
* Images need to be padded by at least a single voxel all around the image (this to ensure that the images are "periodic")
  * The size of the padded border needs to be larger than the size of the smoothing kernel.
  * The smoothing can be disabled if the smoothing happens as part of the preprocessing of the input images.
  * For typical skull-stripped images at CBICA, a linear smoothing is needed (or use whatever CLAIRE does by default) since CBICA uses non-linear smoothing.
  * Typical smoothing kernel should be 2x2x2 (current default is 1x1x1).
* Input image(s) should **always** be affine-ly registered with each other. CLAIRE assumes registration done to T1, not T1Gd as with CBICA.
* Typical runtime commands are highlighted in [Examples](doc/README-RUNME.md)

## License
Read the [LICENSE](https://github.com/andreasmang/claire/tree/master/LICENSE) file for more details.
