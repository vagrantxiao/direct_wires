create_pblock axi_leaf
add_cells_to_pblock [get_pblocks axi_leaf] [get_cells -quiet [list floorplan_static_i/axi_leaf]]
resize_pblock [get_pblocks axi_leaf] -add {SLICE_X37Y0:SLICE_X45Y179}
resize_pblock [get_pblocks axi_leaf] -add {DSP48E2_X7Y0:DSP48E2_X8Y71}
resize_pblock [get_pblocks axi_leaf] -add {RAMB18_X5Y0:RAMB18_X5Y71}
resize_pblock [get_pblocks axi_leaf] -add {RAMB36_X5Y0:RAMB36_X5Y35}
set_property SNAPPING_MODE ON [get_pblocks axi_leaf]


create_pblock p_X3Y0
add_cells_to_pblock [get_pblocks p_X3Y0] [get_cells -quiet [list floorplan_static_i/pe_empty_X3Y0/inst]]
resize_pblock [get_pblocks p_X3Y0] -add {SLICE_X70Y1:SLICE_X96Y59}
resize_pblock [get_pblocks p_X3Y0] -add {DSP48E2_X14Y2:DSP48E2_X17Y23}
resize_pblock [get_pblocks p_X3Y0] -add {IOB_X0Y26:IOB_X1Y51}
resize_pblock [get_pblocks p_X3Y0] -add {RAMB18_X9Y2:RAMB18_X12Y23}
resize_pblock [get_pblocks p_X3Y0] -add {RAMB36_X9Y1:RAMB36_X12Y11}
set_property SNAPPING_MODE ON [get_pblocks p_X3Y0]


create_pblock p_X2Y0
add_cells_to_pblock [get_pblocks p_X2Y0] [get_cells -quiet [list floorplan_static_i/pe_empty_X2Y0/inst]]
resize_pblock [get_pblocks p_X2Y0] -add {SLICE_X46Y0:SLICE_X69Y59}
resize_pblock [get_pblocks p_X2Y0] -add {DSP48E2_X9Y0:DSP48E2_X13Y23}
resize_pblock [get_pblocks p_X2Y0] -add {RAMB18_X6Y0:RAMB18_X8Y23}
resize_pblock [get_pblocks p_X2Y0] -add {RAMB36_X6Y0:RAMB36_X8Y11}
set_property SNAPPING_MODE ON [get_pblocks p_X2Y0]


create_pblock p_X3Y1
add_cells_to_pblock [get_pblocks p_X3Y1] [get_cells -quiet [list floorplan_static_i/pe_empty_X3Y1/inst]]
resize_pblock [get_pblocks p_X3Y1] -add {SLICE_X70Y61:SLICE_X96Y119}
resize_pblock [get_pblocks p_X3Y1] -add {DSP48E2_X14Y26:DSP48E2_X17Y47}
resize_pblock [get_pblocks p_X3Y1] -add {IOB_X1Y78:IOB_X1Y103}
resize_pblock [get_pblocks p_X3Y1] -add {RAMB18_X9Y26:RAMB18_X12Y47}
resize_pblock [get_pblocks p_X3Y1] -add {RAMB36_X9Y13:RAMB36_X12Y23}
set_property SNAPPING_MODE ON [get_pblocks p_X3Y1]



create_pblock p_X2Y1
add_cells_to_pblock [get_pblocks p_X2Y1] [get_cells -quiet [list floorplan_static_i/pe_empty_X2Y1/inst]]
resize_pblock [get_pblocks p_X2Y1] -add {SLICE_X46Y60:SLICE_X69Y119}
resize_pblock [get_pblocks p_X2Y1] -add {DSP48E2_X9Y24:DSP48E2_X13Y47}
resize_pblock [get_pblocks p_X2Y1] -add {RAMB18_X6Y24:RAMB18_X8Y47}
resize_pblock [get_pblocks p_X2Y1] -add {RAMB36_X6Y12:RAMB36_X8Y23}
set_property SNAPPING_MODE ON [get_pblocks p_X2Y1]




create_pblock p_X3Y2
add_cells_to_pblock [get_pblocks p_X3Y2] [get_cells -quiet [list floorplan_static_i/pe_empty_X3Y2/inst]]
resize_pblock [get_pblocks p_X3Y2] -add {SLICE_X70Y121:SLICE_X96Y179}
resize_pblock [get_pblocks p_X3Y2] -add {DSP48E2_X14Y50:DSP48E2_X17Y71}
resize_pblock [get_pblocks p_X3Y2] -add {IOB_X1Y130:IOB_X1Y155}
resize_pblock [get_pblocks p_X3Y2] -add {RAMB18_X9Y50:RAMB18_X12Y71}
resize_pblock [get_pblocks p_X3Y2] -add {RAMB36_X9Y25:RAMB36_X12Y35}
resize_pblock [get_pblocks p_X3Y2] -add {SYSMONE4_X0Y0:SYSMONE4_X0Y0}
set_property SNAPPING_MODE ON [get_pblocks p_X3Y2]


create_pblock p_X2Y2
add_cells_to_pblock [get_pblocks p_X2Y2] [get_cells -quiet [list floorplan_static_i/pe_empty_X2Y2/inst]]
resize_pblock [get_pblocks p_X2Y2] -add {SLICE_X46Y120:SLICE_X69Y179}
resize_pblock [get_pblocks p_X2Y2] -add {DSP48E2_X9Y48:DSP48E2_X13Y71}
resize_pblock [get_pblocks p_X2Y2] -add {RAMB18_X6Y48:RAMB18_X8Y71}
resize_pblock [get_pblocks p_X2Y2] -add {RAMB36_X6Y24:RAMB36_X8Y35}
set_property SNAPPING_MODE ON [get_pblocks p_X2Y2]



create_pblock p_X0Y3
add_cells_to_pblock [get_pblocks p_X0Y3] [get_cells -quiet [list floorplan_static_i/pe_empty_X0Y3/inst]]
resize_pblock [get_pblocks p_X0Y3] -add {CLOCKREGION_X0Y3:CLOCKREGION_X0Y3}
set_property SNAPPING_MODE ON [get_pblocks p_X0Y3]


create_pblock p_X1Y3
add_cells_to_pblock [get_pblocks p_X1Y3] [get_cells -quiet [list floorplan_static_i/pe_empty_X1Y3/inst]]
resize_pblock [get_pblocks p_X1Y3] -add {SLICE_X29Y180:SLICE_X45Y239}
resize_pblock [get_pblocks p_X1Y3] -add {DSP48E2_X6Y72:DSP48E2_X8Y95}
resize_pblock [get_pblocks p_X1Y3] -add {RAMB18_X4Y72:RAMB18_X5Y95}
resize_pblock [get_pblocks p_X1Y3] -add {RAMB36_X4Y36:RAMB36_X5Y47}
set_property SNAPPING_MODE ON [get_pblocks p_X1Y3]

create_pblock p_X2Y3
add_cells_to_pblock [get_pblocks p_X2Y3] [get_cells -quiet [list floorplan_static_i/pe_empty_X2Y3/inst]]
resize_pblock [get_pblocks p_X2Y3] -add {SLICE_X46Y180:SLICE_X69Y239}
resize_pblock [get_pblocks p_X2Y3] -add {DSP48E2_X9Y72:DSP48E2_X13Y95}
resize_pblock [get_pblocks p_X2Y3] -add {RAMB18_X6Y72:RAMB18_X8Y95}
resize_pblock [get_pblocks p_X2Y3] -add {RAMB36_X6Y36:RAMB36_X8Y47}
set_property SNAPPING_MODE ON [get_pblocks p_X2Y3]


create_pblock p_X3Y3
add_cells_to_pblock [get_pblocks p_X3Y3] [get_cells -quiet [list floorplan_static_i/pe_empty_X3Y3/inst]]
resize_pblock [get_pblocks p_X3Y3] -add {SLICE_X70Y181:SLICE_X96Y239}
resize_pblock [get_pblocks p_X3Y3] -add {DSP48E2_X14Y74:DSP48E2_X17Y95}
resize_pblock [get_pblocks p_X3Y3] -add {IOB_X0Y182:IOB_X1Y207}
resize_pblock [get_pblocks p_X3Y3] -add {RAMB18_X9Y74:RAMB18_X12Y95}
resize_pblock [get_pblocks p_X3Y3] -add {RAMB36_X9Y37:RAMB36_X12Y47}
set_property SNAPPING_MODE ON [get_pblocks p_X3Y3]


create_pblock p_X0Y4
add_cells_to_pblock [get_pblocks p_X0Y4] [get_cells -quiet [list floorplan_static_i/pe_empty_X0Y4/inst]]
resize_pblock [get_pblocks p_X0Y4] -add {CLOCKREGION_X0Y4:CLOCKREGION_X0Y4}
set_property SNAPPING_MODE ON [get_pblocks p_X0Y4]



create_pblock p_X1Y4
add_cells_to_pblock [get_pblocks p_X1Y4] [get_cells -quiet [list floorplan_static_i/pe_empty_X1Y4/inst]]
resize_pblock [get_pblocks p_X1Y4] -add {SLICE_X29Y240:SLICE_X45Y299}
resize_pblock [get_pblocks p_X1Y4] -add {DSP48E2_X6Y96:DSP48E2_X8Y119}
resize_pblock [get_pblocks p_X1Y4] -add {RAMB18_X4Y96:RAMB18_X5Y119}
resize_pblock [get_pblocks p_X1Y4] -add {RAMB36_X4Y48:RAMB36_X5Y59}
set_property SNAPPING_MODE ON [get_pblocks p_X1Y4]


create_pblock p_X2Y4
add_cells_to_pblock [get_pblocks p_X2Y4] [get_cells -quiet [list floorplan_static_i/pe_empty_X2Y4/inst]]
resize_pblock [get_pblocks p_X2Y4] -add {SLICE_X46Y240:SLICE_X69Y299}
resize_pblock [get_pblocks p_X2Y4] -add {DSP48E2_X9Y96:DSP48E2_X13Y119}
resize_pblock [get_pblocks p_X2Y4] -add {RAMB18_X6Y96:RAMB18_X8Y119}
resize_pblock [get_pblocks p_X2Y4] -add {RAMB36_X6Y48:RAMB36_X8Y59}
set_property SNAPPING_MODE ON [get_pblocks p_X2Y4]


create_pblock p_X3Y4
add_cells_to_pblock [get_pblocks p_X3Y4] [get_cells -quiet [list floorplan_static_i/pe_empty_X3Y4/inst]]
resize_pblock [get_pblocks p_X3Y4] -add {SLICE_X70Y240:SLICE_X96Y299}
resize_pblock [get_pblocks p_X3Y4] -add {DSP48E2_X14Y96:DSP48E2_X17Y119}
resize_pblock [get_pblocks p_X3Y4] -add {GTHE4_CHANNEL_X1Y4:GTHE4_CHANNEL_X1Y7}
resize_pblock [get_pblocks p_X3Y4] -add {GTHE4_COMMON_X1Y1:GTHE4_COMMON_X1Y1}
resize_pblock [get_pblocks p_X3Y4] -add {IOB_X0Y208:IOB_X0Y231}
resize_pblock [get_pblocks p_X3Y4] -add {RAMB18_X9Y96:RAMB18_X12Y119}
resize_pblock [get_pblocks p_X3Y4] -add {RAMB36_X9Y48:RAMB36_X12Y59}
set_property SNAPPING_MODE ON [get_pblocks p_X3Y4]



create_pblock p_X0Y5
add_cells_to_pblock [get_pblocks p_X0Y5] [get_cells -quiet [list floorplan_static_i/pe_empty_X0Y5/inst]]
resize_pblock [get_pblocks p_X0Y5] -add {CLOCKREGION_X0Y5:CLOCKREGION_X0Y5}
set_property SNAPPING_MODE ON [get_pblocks p_X0Y5]


create_pblock p_X1Y5
add_cells_to_pblock [get_pblocks p_X1Y5] [get_cells -quiet [list floorplan_static_i/pe_empty_X1Y5/inst]]
resize_pblock [get_pblocks p_X1Y5] -add {SLICE_X29Y300:SLICE_X45Y359}
resize_pblock [get_pblocks p_X1Y5] -add {DSP48E2_X6Y120:DSP48E2_X8Y143}
resize_pblock [get_pblocks p_X1Y5] -add {RAMB18_X4Y120:RAMB18_X5Y143}
resize_pblock [get_pblocks p_X1Y5] -add {RAMB36_X4Y60:RAMB36_X5Y71}
set_property SNAPPING_MODE ON [get_pblocks p_X1Y5]


create_pblock p_X2Y5
add_cells_to_pblock [get_pblocks p_X2Y5] [get_cells -quiet [list floorplan_static_i/pe_empty_X2Y5/inst]]
resize_pblock [get_pblocks p_X2Y5] -add {SLICE_X46Y300:SLICE_X69Y359}
resize_pblock [get_pblocks p_X2Y5] -add {DSP48E2_X9Y120:DSP48E2_X13Y143}
resize_pblock [get_pblocks p_X2Y5] -add {RAMB18_X6Y120:RAMB18_X8Y143}
resize_pblock [get_pblocks p_X2Y5] -add {RAMB36_X6Y60:RAMB36_X8Y71}
set_property SNAPPING_MODE ON [get_pblocks p_X2Y5]


create_pblock p_X3Y5
add_cells_to_pblock [get_pblocks p_X3Y5] [get_cells -quiet [list floorplan_static_i/pe_empty_X3Y5/inst]]
resize_pblock [get_pblocks p_X3Y5] -add {SLICE_X70Y300:SLICE_X96Y359}
resize_pblock [get_pblocks p_X3Y5] -add {DSP48E2_X14Y120:DSP48E2_X17Y143}
resize_pblock [get_pblocks p_X3Y5] -add {GTHE4_CHANNEL_X1Y8:GTHE4_CHANNEL_X1Y11}
resize_pblock [get_pblocks p_X3Y5] -add {GTHE4_COMMON_X1Y2:GTHE4_COMMON_X1Y2}
resize_pblock [get_pblocks p_X3Y5] -add {IOB_X0Y232:IOB_X0Y255}
resize_pblock [get_pblocks p_X3Y5] -add {RAMB18_X9Y120:RAMB18_X12Y143}
resize_pblock [get_pblocks p_X3Y5] -add {RAMB36_X9Y60:RAMB36_X12Y71}
set_property SNAPPING_MODE ON [get_pblocks p_X3Y5]

create_pblock p_X0Y6
add_cells_to_pblock [get_pblocks p_X0Y6] [get_cells -quiet [list floorplan_static_i/pe_empty_X0Y6/inst]]
resize_pblock [get_pblocks p_X0Y6] -add {CLOCKREGION_X0Y6:CLOCKREGION_X0Y6}
set_property SNAPPING_MODE ON [get_pblocks p_X0Y6]


create_pblock p_X1Y6
add_cells_to_pblock [get_pblocks p_X1Y6] [get_cells -quiet [list floorplan_static_i/pe_empty_X1Y6/inst]]
resize_pblock [get_pblocks p_X1Y6] -add {SLICE_X29Y360:SLICE_X45Y419}
resize_pblock [get_pblocks p_X1Y6] -add {DSP48E2_X6Y144:DSP48E2_X8Y167}
resize_pblock [get_pblocks p_X1Y6] -add {RAMB18_X4Y144:RAMB18_X5Y167}
resize_pblock [get_pblocks p_X1Y6] -add {RAMB36_X4Y72:RAMB36_X5Y83}
set_property SNAPPING_MODE ON [get_pblocks p_X1Y6]


create_pblock p_X2Y6
add_cells_to_pblock [get_pblocks p_X2Y6] [get_cells -quiet [list floorplan_static_i/pe_empty_X2Y6/inst]]
resize_pblock [get_pblocks p_X2Y6] -add {SLICE_X46Y360:SLICE_X69Y419}
resize_pblock [get_pblocks p_X2Y6] -add {DSP48E2_X9Y144:DSP48E2_X13Y167}
resize_pblock [get_pblocks p_X2Y6] -add {RAMB18_X6Y144:RAMB18_X8Y167}
resize_pblock [get_pblocks p_X2Y6] -add {RAMB36_X6Y72:RAMB36_X8Y83}
set_property SNAPPING_MODE ON [get_pblocks p_X2Y6]


create_pblock p_X3Y6
add_cells_to_pblock [get_pblocks p_X3Y6] [get_cells -quiet [list floorplan_static_i/pe_empty_X3Y6/inst]]
resize_pblock [get_pblocks p_X3Y6] -add {SLICE_X70Y360:SLICE_X96Y419}
resize_pblock [get_pblocks p_X3Y6] -add {DSP48E2_X14Y144:DSP48E2_X17Y167}
resize_pblock [get_pblocks p_X3Y6] -add {GTHE4_CHANNEL_X1Y12:GTHE4_CHANNEL_X1Y15}
resize_pblock [get_pblocks p_X3Y6] -add {GTHE4_COMMON_X1Y3:GTHE4_COMMON_X1Y3}
resize_pblock [get_pblocks p_X3Y6] -add {IOB_X0Y256:IOB_X0Y279}
resize_pblock [get_pblocks p_X3Y6] -add {RAMB18_X9Y144:RAMB18_X12Y167}
resize_pblock [get_pblocks p_X3Y6] -add {RAMB36_X9Y72:RAMB36_X12Y83}
set_property SNAPPING_MODE ON [get_pblocks p_X3Y6]





set_property DONT_TOUCH true [get_cells floorplan_static_i/axi_leaf]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/axi_leaf]
set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X3Y0/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X3Y0/inst]
set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X2Y0/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X2Y0/inst]


set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X3Y1/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X3Y1/inst]
set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X2Y1/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X2Y1/inst]


set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X3Y2/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X3Y2/inst]
set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X2Y2/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X2Y2/inst]


set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X0Y3/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X0Y3/inst]
set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X1Y3/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X1Y3/inst]
set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X2Y3/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X2Y3/inst]
set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X3Y3/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X3Y3/inst]


set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X0Y4/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X0Y4/inst]
set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X1Y4/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X1Y4/inst]
set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X2Y4/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X2Y4/inst]
set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X3Y4/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X3Y4/inst]


set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X0Y5/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X0Y5/inst]
set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X1Y5/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X1Y5/inst]
set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X2Y5/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X2Y5/inst]
set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X3Y5/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X3Y5/inst]


set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X0Y6/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X0Y6/inst]
set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X1Y6/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X1Y6/inst]
set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X2Y6/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X2Y6/inst]
set_property DONT_TOUCH true [get_cells floorplan_static_i/pe_empty_X3Y6/inst]
set_property HD.RECONFIGURABLE true [get_cells floorplan_static_i/pe_empty_X3Y6/inst]







