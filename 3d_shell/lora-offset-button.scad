cutout_size = 2.6;
offset_length = 8;
main_height = 17;

difference() {
  union() {
    cube([cutout_size+2,3+offset_length,3.4]);
    translate([-2,offset_length,0]) {
      cube([cutout_size+4,3,main_height-3]);
      translate([2,0,0]) cube([cutout_size+2,3,main_height]);
    }
    translate([0,-2,main_height-1])
      cube([cutout_size+2,2+offset_length,1]);
    translate([-2,-2,main_height-1]) hull() {
      translate([2,0,0]) cube([3,5+offset_length,1]);
      translate([0,0,-3]) cube([0.4,5+offset_length,1]);
    }
    //translate([0,-3,11])
      //#cube([cutout_size+2,15,1]);
  }
  
  // cutout slot
  translate([-0.05+2,1+cutout_size/2-0.25,-0.01])
      cube([cutout_size+2.1,0.5,2.4]);
  // cutout
  translate([1,1,0]) #hull() {
    translate([0,0,-0.01]) #cube([cutout_size, cutout_size, 2]);
    translate([-0.1,-0.1,2]) #cube([cutout_size+0.2, cutout_size+0.2, 0.5]);
    
    translate([1.25,1.25,0]) cylinder(h=3.2,r=0.01, $fn=30);
  }
}
