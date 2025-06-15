############################################
### Setup source groups                  ###
############################################

# Define a function to group files into source groups based on their relative file paths.
function(group_files relative_file_path_list pre_group_name)
    foreach(relative_file_path IN LISTS relative_file_path_list)
        # Get the path of the file relative to the current source directory.
        get_filename_component(file_path "${relative_file_path}" PATH)
        
        # Remove the current source directory prefix from the file path to get the group path.
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" group_path "${file_path}")
        
        # Replace forward slashes with backslashes to conform to source_group requirements.
        string(REPLACE "/" "\\" group_path_slash "${group_path}")
        
        # Define the source group for the current file.
        source_group("${pre_group_name}/${group_path_slash}" FILES "${relative_file_path}")
    endforeach()
endfunction(group_files)
