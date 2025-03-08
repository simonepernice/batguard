/*
 * batguard a laptop battery charge manager
 * 
 * Copyright (C) 2025 Simone Pernice pernice@libero.it 
 * 
 * This file is part of batguard.
 *
 * batguard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * batguard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
 
epson = 0.01;
wall = [0.4, 0.4, 0.2];

relayDims = [18.9, 15.2, 14.9, 0.8, 3.5];
terminalBlockDims = [8.0, 15.6, 10.0, 1.0, 3.7];
usbConnectorDims = [14.9, 12.2, 4.5, 18.9, 3.5];
pcbDims = [46.6, 16.1, 1.6, 0.2];
eleCableDims = [8, 8, 50, 1];

thickness = 4 * wall.x;
gap = 0.4;
caseDims = [49.5 + eleCableDims.x , 17.0, 19.3, 2];
caseOvlp = 3;
caseGap = 0.1;

$fn = 64;

//0 no slice
//1 slicing parallel to x/z plane
//2 slicing parallel to x/y plane top
//3 slicing parallel to x/y plane bottom
//4 slicing parallel to z/y plane bottom
slice = 0; 

//false no relay
//true show relay 
showRelay = false;

//0 single case 
//1 top case
//2 bottom case
//3 bottom case upside down for printing
//4 top and bottom with 0.1 gap
showCase = 3;

module bottomCase ()
{
    difference ()
    {
        usdRelayroundedCubeCase ();
        cs = 100;
        translate ([0, 0, -cs/2+pcbDims.z+usbConnectorDims.z]) cube (cs, center = true);        
        topCase (thickness/2 + caseGap);
    }
}

module topCase (coltck = thickness/2 - caseGap)
{   
    difference ()
    {
        color ("DarkSlateGray") union ()
        {
            translate ([0,0,pcbDims.z+usbConnectorDims.z-epson]) roundedCollar (caseDims + [0,0,caseOvlp-caseDims.z,0], coltck);
            difference ()
            {                
                usdRelayroundedCubeCase ();
                cs = 100;
                translate ([0, 0, cs/2+pcbDims.z+usbConnectorDims.z]) cube (cs, center = true);
            }
        }
        translate ([0,0,gap]) usbRelay (usbConnectorDims + gap* [0, 2, 2, 0, 0], eleCableDims + 2*gap*[1,1,1,0]);
    }
}
    
module usdRelayroundedCubeCase (dims = caseDims)
{
    difference ()
    {
        color ("DarkSlateGray") union ()
        {
            translate ([0, 0, dims.z/2 + thickness]) difference ()
            {
                roundedCube (dims + 2 * thickness * [1, 1, 1, 1]);
                roundedCube (dims);
            }
            translate ([-caseDims.x/2+pcbDims.x+1.3*thickness, 0, terminalBlockDims [4]/2+thickness-epson]) cube ([thickness, caseDims.y, terminalBlockDims [4]], center = true);
        }
        translate ([0,0,gap]) usbRelay (usbConnectorDims + gap* [0, 2, 2, 0, 0], eleCableDims + 2*gap*[1,1,1,0]);
    }
}

module roundedCollar (dims, thick)
{
    translate ([0, 0, dims.z/2]) difference ()
    {
        roundedCube (dims + 2 * thick * [1, 1, 0, 1]);
        roundedCube (dims + 2 * thick * [0, 0, 1, 0]);
    }
}

module roundedCube (dims)
{
    width = dims [0];
    depth = dims [1];
    height = dims [2];
    curv = dims [3];

    hull () for (i = [-1, 1]) for (j = [-1, 1]) translate ([i*(width - curv)/2, j*(depth - curv)/2, 0]) cylinder (d = curv, h = height, center = true);
}
module relay (dims = relayDims)
{
    width = dims [0];
    depth = dims [1];
    height = dims [2];
    pindia = dims [3];
    pinhei = dims [4];
    
    translate ([0, 0, height/2])
    {
        color ("DeepSkyBlue") cube ([width, depth, height], center = true);
        for (i = [-1, 1]) for (j = [-1, 1]) translate ([i*(width/2 - 3.3), j*(depth/2 -1.9), -height/2-pinhei/2]) color ("silver") cylinder (d = pindia, h = pinhei, center = true);
        translate ([-(width/2 - 0.8), 0, -height/2-pinhei/2]) color ("silver") cylinder (d = pindia, h = pinhei, center = true);
    }
 }
 
 module terminalBlock (dims = terminalBlockDims)
 {
    width = dims [0];
    depth = dims [1];
    height = dims [2];
    pindia = dims [3];
    pinhei = dims [4];
    
    translate ([0, 0, height/2])
    {
        color ("DodgerBlue") cube ([width, depth, height], center = true);
        for (i = [-1, 0, 1]) translate ([0, i*(depth/2 - 3), -height/2-pinhei/2]) color ("silver") cylinder (d = pindia, h = pinhei, center = true);
    }
 }   
 
 module usbConnector (dims = usbConnectorDims)
  {
    width = dims [0];
    depth = dims [1];
    height = dims [2];
    widthin = dims [3];
    heightin = dims [4];
    
    color ("LightGrey") translate ([0, 0, heightin/2+height-heightin])
    {
        cube ([widthin, depth, heightin], center = true);
        translate ([-(widthin-width)/2, 0, -(height-heightin)/2-epson]) cube ([width, depth, height], center = true);
    }
 }   
 
 module pcb (dims = pcbDims)
 {
    width = dims [0];
    depth = dims [1];
    height = dims [2];

    translate ([0, 0, height/2])
    {
        color ("green") cube ([width, depth, height], center = true);
    }
}

module eleCable (dims = eleCableDims)
{
    diameter = dims [0];
    length = dims [2];
    translate ([0, 0, diameter/2]) rotate ([90,0,0]) color ("LightCyan") cylinder (d = diameter, h = length, center = true);
}

module usbRelay (ucd = usbConnectorDims, eld = eleCableDims)
{
    translate ([-(eleCableDims.x+eleCableDims[3])/2, 0, thickness + terminalBlockDims[4]-pcbDims.z]) 
    {
        pcb ();
        translate ([(pcbDims.x-relayDims.x)/2-terminalBlockDims.x - pcbDims [3], 0, pcbDims [3] + pcbDims.z]) relay ();
        translate ([(pcbDims.x-terminalBlockDims.x)/2, 0, pcbDims [3] + pcbDims.z]) terminalBlock ();
        translate ([(-pcbDims.x-ucd.x+ucd[4])/2, 0, pcbDims [3] + pcbDims.z-(ucd.z-ucd[4])]) usbConnector (ucd);
        translate ([(pcbDims.x + eld.x)/2+eld [3], 0, terminalBlockDims.z*2/3]) eleCable (eld);
    }
}

difference ()
{
    union ()
    {
        if (showRelay == true) usbRelay (); 
        if (showCase == 0) usdRelayroundedCubeCase();
        else if (showCase == 1) topCase ();
        else if (showCase == 2) bottomCase ();
        else if (showCase == 3) rotate ([180,0,0]) translate ([0,0,-pcbDims.z-usbConnectorDims.z]) bottomCase ();
        else if (showCase == 4) 
        {
            topCase ();
            translate ([0,0,.1]) bottomCase ();
        }
        else echo ("Wrong showCase Number");
    }
    if (slice == 1) translate ([-50,0,-1]) cube (100);
    else if (slice == 2) translate ([0,0,-50+caseDims.z/2]) cube (100, center = true);
    else if (slice == 3) translate ([0,0,+50+caseDims.z/2]) cube (100, center = true);
    else if (slice == 4) translate ([-50,0,0]) cube (100, center = true);
    else echo ("Wrong Slice Number");
}
