drop table if exists Items; 
drop table if exists Users; 
drop table if exists Descriptions;
drop table if exists Bids;
drop table if exists Category; 

create table Items(
    ItemID INTEGER PRIMARY KEY, 
    iName text NOT NULL,
    Currently REAL NOT NULL,
    UserID text NOT NULL,
    FOREIGN KEY (UserID) References Users(UserID)
);

create table Category(
    ItemID INTEGER,
    Category text NOT NULL,
    FOREIGN KEY (ItemID) References Items(ItemID)
);
create table Users(
    UserID TEXT PRIMARY KEY,
    Rating Integer,
    Loc Text, 
    Country Text
);
create table Descriptions(
    ItemID integer, 
    iDesc TEXT, 
    First_Bid REAL NOT NULL,
    Start_Time smalldatetime NOT NULL,              
    End_Time smalldatetime NOT NULL, 
    NumOfBids integer NOT NULL, 
    Buy_Price Real,
    FOREIGN KEY (ItemID) References Items (ItemID)   
);
create table Bids(
    ItemID INTEGER,
    UserID TEXT,
    BTime smalldatetime, 
    Amount REAL,
    FOREIGN KEY (UserID) References Users (UserID),
    FOREIGN KEY (ItemID) References Items (ItemID)
);