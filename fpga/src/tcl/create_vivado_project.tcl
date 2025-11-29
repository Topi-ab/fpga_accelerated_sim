# ================================================================
#  Find repository root
# ================================================================
set repo_root [string trim [exec git rev-parse --show-toplevel]]

# ================================================================
#  RTL Directory
# ================================================================
set rtl_dir [file join $repo_root fpga src rtl]
set tcl_dir [file join $repo_root fpga src tcl]
set xdc_dir [file join $repo_root fpga src xdc]

set proj_name "linkruncca_accelerator"
set proj_dir  [file join $repo_root fpga_proj]

# ================================================================
#  Delete old project directory
# ================================================================
if {[file exists $proj_dir]} {
    puts "Deleting old project directory: $proj_dir"
    file delete -force $proj_dir
}

# ================================================================
#  Create project
# ================================================================
create_project $proj_name $proj_dir -part xck26-sfvc784-2LV-c

# Refresh project directory handle from Vivado
set proj_dir [get_property directory [current_project]]

set_property board_part          "xilinx.com:kv260_som:part0:1.4" -objects [current_project]
set_property enable_vhdl_2008    1                                 -objects [current_project]
set_property platform.board_id   "kv260_som"                        -objects [current_project]
set_property target_language     "VHDL"                             -objects [current_project]


# ================================================================
#  Recursive file search (Vivado-compatible!)
# ================================================================
proc glob_recursive {root patterns} {
    set results {}

    # List directory contents
    foreach item [glob -nocomplain -directory $root *] {

        if {[file isdirectory $item]} {
            # Recurse into subdirs
            set sub [glob_recursive $item $patterns]
            if {[llength $sub] > 0} {
                set results [concat $results $sub]
            }

        } else {
            # Match file against patterns
            foreach pat $patterns {
                if {[string match -nocase $pat [file tail $item]]} {
                    lappend results $item
                }
            }
        }
    }
    return $results
}

# ================================================================
#  Collect RTL files
# ================================================================
set patterns [list *.v *.sv *.vhd *.vhdl]
set file_list [glob_recursive $rtl_dir $patterns]

puts "Found [llength $file_list] RTL files"


# ================================================================
#  Add RTL files to project
# ================================================================
foreach f $file_list {
    puts "Adding: $f"
    add_files -norecurse $f

    # Assign VHDL 2008 only to VHDL files
    if {[string match -nocase *.vhd $f] || [string match -nocase *.vhdl $f]} {
        set_property file_type {VHDL 2008} [get_files $f]
    }
}

puts "RTL import complete."

foreach f [glob -nocomplain -directory $xdc_dir *.xdc] {
    puts "Adding XDC: $f"
    add_files -fileset constrs_1 -norecurse $f
}

puts "XDC import complete."

set_property file_type VHDL [get_files  ${rtl_dir}/axi_passthrough_stub_v1_0.vhd]

source ${tcl_dir}/zynq_bd.tcl
