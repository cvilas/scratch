local base_dim = 600

system_name = "Test System"

config_table = {
  window_width = base_dim + 200,
  window_height = base_dim,
  num_enemies = 15,
  num_levels = 10,
  nested_table = {
    my_key="My Value",
    my_key2="My second value"
  },
}

personnel_records = {
  { name = "John Doe", age = 40, coordinates = {51.504497, -0.467789} },
  { name = "Jane Smith", age = 65, coordinates = {12.967917, 80.236271} }
}

print("num records in config_table: " ..#config_table)
print("num records in personnel_records: " ..#personnel_records)
print("window width: " ..config_table.window_height)
print("First name in personnel records : ".. personnel_records[1].name)
