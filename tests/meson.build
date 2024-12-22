catch_dep = dependency('catch2-with-main', version:'>3.0.0', required : true)


foreach t : ['pcb_lpp_lob', 'ftdi']
    exe=executable(t, t+'/main.cpp', dependencies : [lpp_boards_pp_dep, catch_dep])
    test(t+'_test', exe)
endforeach
