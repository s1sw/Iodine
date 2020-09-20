sources = [
  'lexer.cpp',
  'parser.cpp'
]

iodine_parser_include_dir = include_directories('.')

iodine_parser = static_library('iodine_parser', sources) 
iodine_parser_dep = declare_dependency(link_with: iodine_parser, include_directories: iodine_parser_include_dir)
