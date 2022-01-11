    program test_cpu_time
    real :: start, finish
    call cpu_time(start)

    integer :: i

    do 10 i = 1, 100
10  continue

    call cpu_time(finish)
    print '("Time = ",f6.3," seconds.")',finish-start
    end program test_cpu_time