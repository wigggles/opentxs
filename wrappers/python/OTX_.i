/* The swig module name affects the build, and there were issues with 
   conflicts by having them all named opentxs (from the original monolithic
   opentxs.i). 
   This has been named opentxs and may be in use by others.
   Would the name pyopentxs be better?
*/
%module(directors=1) opentxs

%include "../opentxs.i"
