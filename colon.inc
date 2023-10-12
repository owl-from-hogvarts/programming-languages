%ifndef colon_inc
%define colon_inc

    %macro colon 2
    %2:
        %ifndef last_entry
            dq 0
        %else
            dq last_entry
        %endif
        %define last_entry %2
        db %1, 0
    %endmacro

%endif

