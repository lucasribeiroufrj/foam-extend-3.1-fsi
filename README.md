# foam-extend-3.1-fsi

##A non-Newtonian-FSI solver
The purpose of this repository is to support an extension of a (_Newtonian_)FSI solver. This extension allows one to simulate any non-Newtonian fluid flow, already implemented in the foam-extended-3.1, using this solver.

##Where did the idea come from?  
The repository started as a clone of the [Extend-bazaar/Toolkits/Fluid-structure interaction]
(https://openfoamwiki.net/index.php/Extend-bazaar/Toolkits/Fluid-structure_interaction#Install_on_foam-extend_3.1).

##How to use it?  
1) Download and install foam-extend-3.1 [(excellent source of information: www.idurun.com)](http://www.idurun.com/2015/01/24/install-foam-extend-3-1-on-centos-7/)  
2) Clone this repository, i.e.,  
`$ foam`       # Go to Foam directory, let's put the download at this location  
`$ git clone https://github.com/lucasribeiroufrj/foam-extend-3.1-fsi.git`  
`$ cd foam-extend-3.1-fsi`  
3) Compile it, i.e.,  
`$ cd src`  
`$./Allwmake > log.compilation 2>&1`  
4) Check the compilation log to be sure that the compilation was successful , i.e.,  
`$ cat log.compilation | less`  
5) Copy any case and run in order to test it, e.g.,  
`$ cd ../run`  
`$ cp -rp fsiFoam fsiFoamCopy`  
`$ cd fsiFoamCopy`  
`$ sed -i s/tcsh/sh/g *Links`  
`$ ./removeSerialLinks fluid solid`  
`$ ./makeSerialLinks fluid solid`  
`$ cd fluid`  
`$ ./Allclean`  
`$ ./Allrun`  
.  
Refs.:  
These Instructions was adapted from [openfoamwiki](https://openfoamwiki.net/index.php/Extend-bazaar/Toolkits/Fluid-structure_interaction#Install_on_foam-extend_3.1)
