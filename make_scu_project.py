
import os
import re
import sys

vcxproj_dir = ''


def write_lines_to_vcxproj_file(filename, lines):

    file = open(filename, 'w')

    file.write('\n'.join(lines))

    file.close()


def create_scu_file(scu_filename, cpp_to_include):

    file = open(scu_filename, 'w')

    for cpp_file_entry in cpp_to_include:

        print 'Including ' + cpp_file_entry + ' into: ' + scu_filename

        include_cpp_line = '#include ' + cpp_file_entry

        file.write(include_cpp_line + '\n')

    file.close()


def create_cpp_scu_files(src_cpp_files, scu_files):
    print len(src_cpp_files)

    cpp_portion = len(src_cpp_files) // len(scu_files)

    cpp_modulo = len(src_cpp_files) % len(scu_files)

    create_scu_file(scu_files[0], src_cpp_files[:cpp_portion + cpp_modulo])

    next_cpp_entry_index = cpp_portion + cpp_modulo

    for scu_index in range(1, len(scu_files)):

        create_scu_file(scu_files[scu_index], src_cpp_files[next_cpp_entry_index : next_cpp_entry_index + cpp_portion])

        next_cpp_entry_index += cpp_portion
    


def parse_vcxproj(vcxproj_lines):

    output_vcxproj_lines = []

    cpp_files = []

    cpp_scu_files = []

    build_configurations = []

    for line_index, vcxproj_line in enumerate(vcxproj_lines):

        match_cl_compile_cpp = re.search(r'<ClCompile Include=', vcxproj_line)

        if match_cl_compile_cpp:

            processed_line = vcxproj_line.replace(r'\\', r'\\\\')

            match_cl_compile_cpp = re.search(r'^(\s+)<ClCompile Include=(".+\.cpp")', processed_line)

            if match_cl_compile_cpp:

                cl_item_group_tab = match_cl_compile_cpp.group(1)

                cl_cpp_path = match_cl_compile_cpp.group(2)

                cl_compile_cpp_line = cl_item_group_tab + '<ClCompile Include=' + cl_cpp_path + '>'

                excluded_from_build_line = cl_item_group_tab + '\t' + '<ExcludedFromBuild>' + 'true' + '</ExcludedFromBuild>'

                cl_compile_cpp_line_close_tag = cl_item_group_tab + '</ClCompile>'

                cpp_files.append(cl_cpp_path)

                output_vcxproj_lines.append(cl_compile_cpp_line)

                output_vcxproj_lines.append(excluded_from_build_line)

                output_vcxproj_lines.append(cl_compile_cpp_line_close_tag)

                print cl_cpp_path + ' was excluded from build'

                if line_index < (len(vcxproj_lines) - 1):

                    next_line = vcxproj_lines[line_index + 1]

                    match_cl_item_group_close_tag = re.search(r'</ItemGroup>', next_line)

                    if match_cl_item_group_close_tag:

                        ## add SCU files:

                        for i in range(4):

                            cpp_scu_filename = vcxproj_dir + '\\scu' + str(i + 1) + '.cpp'

                            cpp_scu_files.append(cpp_scu_filename)

                            cl_compile_cpp_scu_line = cl_item_group_tab + '<ClCompile Include="' + cpp_scu_filename + '" />'

                            output_vcxproj_lines.append(cl_compile_cpp_scu_line)

                            print cpp_scu_filename + ' was included to build'

        else:

            output_vcxproj_lines.append(vcxproj_line)

            match_cl_compile_optimization_tag = re.search(r'^(\s+)<Optimization>.+</Optimization>$', vcxproj_line)

            if match_cl_compile_optimization_tag:

                multi_processor_comp_flag_line = '<MultiProcessorCompilation>true</MultiProcessorCompilation>'

                output_vcxproj_lines.append(multi_processor_comp_flag_line)

    create_cpp_scu_files(cpp_files, cpp_scu_files)

    return output_vcxproj_lines




def read_lines_from_vcxproj(filename):
    vcxproj_lines = []

    try:
        file = open(filename, 'r')
        vcxproj_lines = file.read().split('\n')
        file.close()
    except IOError:
        sys.exit('cannot open file' + filename)

    return vcxproj_lines



if len(sys.argv) < 2:
    sys.exit('provide vcxproj file')

src_vcxproj_file = sys.argv[1]

match_src_vcxproj = re.search(r'(.+)\.vcxproj$', src_vcxproj_file)

if not match_src_vcxproj:
    sys.exit('input file is not vcxproj')


dst_vcxproj_file = match_src_vcxproj.group(1) + '-scu' + '.vcxproj'

print 'Creating: ' + dst_vcxproj_file


vcxproj_dir = os.path.dirname(os.path.abspath(src_vcxproj_file))


src_vcxproj_content = read_lines_from_vcxproj(src_vcxproj_file)


dst_vcxproj_content = parse_vcxproj(src_vcxproj_content)


write_lines_to_vcxproj_file(dst_vcxproj_file, dst_vcxproj_content)
