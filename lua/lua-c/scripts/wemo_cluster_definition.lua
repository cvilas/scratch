cluster_definition={
  name="cluster name",
  members={
   {name="sim_wemo0001", x=0.0, y=0.5, z=0.0, rz=0.0},
   {name="sim_wemo0002", x=0.0, y=-0.5, z=0.0, rz=0.0}
  }
}

print("Cluster " .. cluster_definition.name .. " contains " .. 
      #cluster_definition.members .. " robots named " ..
      cluster_definition.members[1].name .. " and " .. 
      cluster_definition.members[2].name)
