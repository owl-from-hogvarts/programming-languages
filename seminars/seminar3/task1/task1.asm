%macro comma_separated 1-*
    %assign i %0
    %rep %0
        db %1
        %if i > 1
            db ","
            %assign i i-1
        %endif
        %rotate 1
    %endrep

%endmacro


comma_separated "hello", "world"


