/*
What we want to do in this program
is first, define the outer dimensions
of a homogeneous solid. Then, we want
to divide the solid into a finite
number of control volumes, or CVs.
Then, what we should be able to do is
specify the boundary conditions on
those CVs (isothermal or constant 
heat flux, denoted 'T' or 'Q' resp.).

We'll start with the assumption of
rectangular prism elements, but we'd 
ideally like to make this generalizable
to other elements in the future.

The temperature or heat flux conditions
ideally should be able to be specified
in a functional form (10*sin(t) or from
an external file like a .csv, but for
now, we will assume that these BCs are
constant. 

The last thing that we'd like to be able
to do is intialize different parts of the
solid at different temperatures using
index ranges in each of the cartesian
coordinate directions.

Since there's a lot of parameters to 
input to start the simulation, we should
probably put all of this information in
some sort of case file that is human-
readable.

Each of the CVs should be a struct with
fields for the temperature state for the
element, the type of BC applied, and 
perhaps the thermal mass of the element,
centroid of the element (in case the index
info is somehow dropped?) or the index of
the element.
*/