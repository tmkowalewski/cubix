As seen in the tkn::tkmeasure class, tkn offers the possibility to convert some measure into different units\n

Currently, tkn handles three unit types: Energy, Time en Lenght. Few other units are included in tkn but that cannot be converted\n\n

The available energy units are the following:  [eV](@ref tkn::tkunit_manager::eV), [keV](@ref tkn::tkunit_manager::keV), [MeV](@ref tkn::tkunit_manager::MeV), [GeV](@ref tkn::tkunit_manager::GeV), [TeV](@ref tkn::tkunit_manager::TeV)\n
The available time units are the following:  [as](@ref tkn::tkunit_manager::as), [fs](@ref tkn::tkunit_manager::fs), [ps](@ref tkn::tkunit_manager::ps), [ns](@ref tkn::tkunit_manager::ns), [us](@ref tkn::tkunit_manager::us), [ms](@ref tkn::tkunit_manager::ms), [s](@ref tkn::tkunit_manager::s), [min](@ref tkn::tkunit_manager::min), [h](@ref tkn::tkunit_manager::h), [d](@ref tkn::tkunit_manager::d), [y](@ref tkn::tkunit_manager::y)\n
The available lenght units are the following:  [am](@ref tkn::tkunit_manager::am), [fm](@ref tkn::tkunit_manager::fm), [pm](@ref tkn::tkunit_manager::pm), [nm](@ref tkn::tkunit_manager::nm), [um](@ref tkn::tkunit_manager::um), [mm](@ref tkn::tkunit_manager::mm), [cm](@ref tkn::tkunit_manager::cm), [m](@ref tkn::tkunit_manager::m)\n
\n
In general, units can be converted only in the scale of a same type. One exception is given for the energy to time conversion. For very low lifetimes, it is common to express the width of a level in energy. This conversion is taken into account in tkn.
