/// The size of a type.
#[derive(Copy, Clone, Debug, PartialEq, Eq, Hash)]
#[allow(missing_docs)]
pub enum Size {
    U8,
    U16,
    U32,
    U64,
}

impl Size {
    /// Return the number of bits this `Size` represents.
    pub fn bits(self) -> u8 {
        match self {
            Self::U8 => 8,
            Self::U16 => 16,
            Self::U32 => 32,
            Self::U64 => 64,
        }
    }

    /// Return the number of bytes in a size.
    ///
    /// A byte is assumed to be 8 bits.
    pub fn bytes(self) -> u8 {
        match self {
            Self::U8 => 1,
            Self::U16 => 2,
            Self::U32 => 4,
            Self::U64 => 8,
        }
    }
}

/// The C data model used on a target.
///
/// See also https://en.cppreference.com/w/c/language/arithmetic_types
#[derive(Copy, Clone, Debug, PartialEq, Eq, Hash)]
#[non_exhaustive]
pub enum CDataModel {
    /// The data model used most commonly on Win16. `long` and `pointer` are 32 bits.
    LP32,
    /// The data model used most commonly on Win32 and 32-bit Unix systems.
    ///
    /// `int`, `long`, and `pointer` are all 32 bits.
    ILP32,
    /// The data model used most commonly on Win64
    ///
    /// `long long`, and `pointer` are 64 bits.
    LLP64,
    /// The data model used most commonly on 64-bit Unix systems
    ///
    /// `long`, and `pointer` are 64 bits.
    LP64,
    /// A rare data model used on early 64-bit Unix systems
    ///
    /// `int`, `long`, and `pointer` are all 64 bits.
    ILP64,
}

impl CDataModel {
    /// The width of a pointer (in the default address space).
    pub fn pointer_width(self) -> Size {
        match self {
            Self::LP32 | Self::ILP32 => Size::U32,
            Self::LLP64 | Self::LP64 | Self::ILP64 => Size::U64,
        }
    }
    /// The size of a C `short`. This is required to be at least 16 bits.
    pub fn short_size(self) -> Size {
        match self {
            Self::LP32 | Self::ILP32 | Self::LLP64 | Self::LP64 | Self::ILP64 => Size::U16,
        }
    }
    /// The size of a C `int`. This is required to be at least 16 bits.
    pub fn int_size(self) -> Size {
        match self {
            Self::LP32 => Size::U16,
            Self::ILP32 | Self::LLP64 | Self::LP64 | Self::ILP64 => Size::U32,
        }
    }
    /// The size of a C `long`. This is required to be at least 32 bits.
    pub fn long_size(self) -> Size {
        match self {
            Self::LP32 | Self::ILP32 | Self::LLP64 | Self::ILP64 => Size::U32,
            Self::LP64 => Size::U64,
        }
    }
    /// The size of a C `long long`. This is required (in C99+) to be at least 64 bits.
    pub fn long_long_size(self) -> Size {
        match self {
            Self::LP32 | Self::ILP32 | Self::LLP64 | Self::ILP64 | Self::LP64 => Size::U64,
        }
    }
    /// The size of a C `float`.
    pub fn float_size(self) -> Size {
        // TODO: this is probably wrong on at least one architecture
        Size::U32
    }
    /// The size of a C `double`.
    pub fn double_size(self) -> Size {
        // TODO: this is probably wrong on at least one architecture
        Size::U64
    }
}
