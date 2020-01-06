$fn=40;
main_h = 28-0.1+2;
base_length = 96;
top_h = 22;

//base();
intersection() {
  //translate([0,0,10]) cube([52,50,30]);
  top();
}
module top() {
  translate([0,0,15.2]) difference() {
    //cube([70,base_length,main_h-15]);
      // above PCB
    union() {
      translate([0,0,0]) hull() {
        cube([70, base_length-0.6, 1]);
        cube([70, base_length-0.6-15, 15]);
      }
      translate([-2, base_length-22,0]) hull() {
        cube([2,6,15]);
        translate([2-0.01,0,0]) #cube([0.01,6,22]);
      }
      translate([0,0,0])
        cube([70, base_length-16, top_h]);
      
      // bolt flange
      translate([0,6,0]) hull() {
        cube([70,20,top_h]);
        translate([-6,0,0]) cylinder(d=12,h=top_h);
        translate([70+6,0,0]) cylinder(d=12,h=top_h);
      }
      
    }

    // bolt thingies
    translate([0,6,0]) {
      translate([-6,0,-0.01]) {
        cylinder(d=4.5,h=top_h-3.2);
        translate([0,0,top_h-3]) cylinder(d=8.25,h=3.2, $fn=6);
      }
      translate([70+6,0,-0.01]) {
        cylinder(d=4.5,h=top_h-3.2);
        translate([0,0,top_h-3]) cylinder(d=8.25,h=3.2, $fn=6);
      }
    }

    translate([(70-56)/2-1,2,-0.01])
      cube([56+1,base_length,15]);
    translate([(70-66)/2,2,-0.01]) 
      cube([66,base_length,12]);
    
    // extra thicc for side USB
    translate([-0.01,base_length-16,-0.01])
      #cube([1.6+1,16,20]);
    
    // non-usb side
    translate([70-1.44,base_length-16,-0.01])
      cube([1.6,16,20]);
    
    // above PCB
    /* translate([2.8/2,base_length-20,-0.01]) hull() {
      cube([66-0.4*2+2, 20, 1]);
      cube([66-0.4*2+2, 20-15, 16]);
    } */
  //55
    
    // GPS
    translate([70/2,38,0]) union() {
      translate([-17/2,12,15-0.02])
        cube([17,42,14.2]);
      
      translate([-40/2,0,15-0.02])
        cube([40,42+16+3,6]);
      translate([-42/2,0,15+2]) hull() {
        cube([42,43,1]);
        translate([1,0,-1.4]) cube([40,43,1.4]);
      }
      translate([-41.5/2,33,15+2]) hull() {
        cube([41.5,43,1]);
        translate([1,0,-1.4]) cube([40-0.5,43,1.4]);
      }
      
      // qwiic
      translate([-7.6/2,-16,14-0.1]) hull() {
        translate([0,10,0]) cube([7.6,7,7.2]);
        cube([7.6,17+12,7]);
      }
    }
    
    // logger
    // basic opening
    translate([70/2-31+3.5,8,15-0.02])
      cube([25,26+4.1+8,6]);
    // thinner section for sd card holder
    translate([70/2-31+3.5+3,8,19.65])
      cube([20,28,2]);
    
    // qwiic notch
    translate([70/2-31+24,6+11.5,12-0.02]) cube([5+8,16+5,9]);

    // sd card thumb opening
    translate([70/2-31+3.5+5,-0.1,17.1-0.02]) hull() {
      translate([-4,0,5]) cube([24,0.01,1]);
      translate([0,0,-1]) cube([16,0.01,1]);
      cube([16,4.1+8+4,5]);
    }
    
    translate([70/2-31+3,8,14-0.02]) hull() {
      //translate([-17/2,0,15-0.02])
       translate([0.25,0,3]) cube([25.5,26+4.1+8,2]);
      translate([0.15,0,3.5]) cube([25.7,4.1,1]);
        translate([0,10,3.5]) cube([26,26+4.1-4,1]);
      //translate([0.5,0,2.5]) cube([26.5,26+4.1,4]);
    }
    
    // USB
    translate([19, -0.1, -1.2])
      cube([9.5,2.2,5]);
    
    // reset button
    translate([41,17,0]) hull() {
      cube([7.5,14,22.1]);
      translate([-0.5,0,0]) cube([8.5,31,19.5]);
    }
  }
}

module base() {
  // +20 for LCD above antenna
  difference() {
    union() {
      cube([70,base_length,main_h]);
      translate([0,base_length-10.2,main_h-0.1]) hull() {
        translate([15,0,0]) cube([30+10,10.2,0.01]);
       translate([18,0,0]) cube([34,10.2,4]);
      }
      
      // bolt flange
      translate([0,6,0]) hull() {
        cube([70,20,main_h]);
        translate([-6,0,0]) cylinder(d=12,h=main_h);
        translate([70+6,0,0]) cylinder(d=12,h=main_h);
      }
      translate([0,base_length-2,0]) hull() {
        translate([-2,-20,0]) cube([70+2,20+2,main_h]);
        /* translate([-6,0,0]) #cylinder(d=12,h=main_h);
        translate([70+6,0,0]) cylinder(d=12,h=main_h); */
      }
    }
    
    // bolt thingies
    translate([0,6,0]) {
      translate([-6,0,-0.1]) {
        #cylinder(d=4.5,h=main_h+0.2);
        translate([0,0,main_h-3]) cylinder(d=8.25,h=3.2, $fn=6);
      }
      translate([70+6,0,0]) {
        #cylinder(d=4.5,h=main_h+0.2);
        translate([0,0,main_h-3]) cylinder(d=8.25,h=3.2, $fn=6);
      }
    }
    translate([0,base_length-6,0]) {
      translate([-6,0,-0.1]) {
        #cylinder(d=4.5,h=main_h+0.2);
        translate([0,0,main_h-3]) #cylinder(d=8.25,h=3.2, $fn=6);
      }
      translate([70+6,0,0]) {
        #cylinder(d=4.5,h=main_h+0.2);
        translate([0,0,main_h-3]) #cylinder(d=8.25,h=3.2, $fn=6);
      }
    }
    
    // base (under the pcb)
    translate([3,2,2])
      cube([64,base_length-4.8,13]);
    // 'duino plate
    //translate([0,0,13+16])
    translate([8,20,13+13])
      cube([54,58,2]);
    
    // top (bottom?) USB
    translate([19, -0.1, 14])
        #cube([9.5,2.2,6]);
    // side USB
    translate([-1,base_length-3-12+4-10,14]) hull() {
      cube([5.2,17,8]);
      //translate([5.1-1,-6,0]) #cube([0.1,17,8]);
    }
    
    // PCB jut-out near antenna
    translate([20.5,base_length-2,13])
      #cube([40,2.2,4]);
    // antenna
    translate([37,base_length-4-20,11])
      #cube([12,4.2+20,10]);
    
    translate([2,2,2]) {
      // pcb and pins above it
      translate([0,0,11]) hull() {
        cube([66,base_length-2.8,0.2]);
        translate([0.4,0,0.01])
          cube([66-0.4*2,base_length-2.8,2.4]);
      }
      // above PCB
      translate([0.4-1,0,13-0.01]) hull() {
        cube([66-0.4*2+2, base_length-2.8, 1]);
        cube([66-0.4*2+2, base_length-2.8-15, 16]);
      }

    }
    
    // LCD slot/cutout
    translate([20,base_length-10.2-0.1,main_h-0.01]) cube([30,10.4,4.2]);

    
    // wipe off the top
    translate([-15,-0.8,15]) cube([130,base_length-3-12-0,15]);
    //4,4
    //translate([7,-0.8,15]) #cube([130,base_length-3-12,15]);
    translate([7,base_length-16.2,15])
      cube([70-7*2,6,16]);
    //cube([70,base_length,main_h]);
    
  /*  // tmp wipe the far side so we can print faster
    translate([60,-2,-1]) cube([100,120,30]);
    // tmp wipe inner corner
      translate([-12,6,-1]) cube([40,60,30]);

    translate([-12,-1,-1]) cube([11,10,30]);
    // 40,9 */
  }
}