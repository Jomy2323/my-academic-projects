package hva;

import hva.states.SeasonState;
public abstract class Tree{

    /** tree's id */
    private String _treeId;

    /** tree's name */
    private String _treeName;

    /** tree's age */
    private int _treeAge;

    /** tree's base cleaning difficulty */
    private int _treeBaseCleaningDifficulty;

    /** tree's current season */
    private SeasonState _treeState;

    /**
     * Constructor
     * @param treeId the tree's id
     * @param treeName the tree's name
     * @param treeAge the tree's age
     * @param treeBaseCleaningDifficulty the tree's base cleaning
     *                                   difficulty
     */
    public Tree(String treeId, String treeName, int treeAge,
     int treeBaseCleaningDifficulty){
        _treeId = treeId;
        _treeName = treeName;
        _treeAge = treeAge*4;
        _treeBaseCleaningDifficulty = treeBaseCleaningDifficulty;
    }

    // Season methods
    public void nextSeason(){
        _treeState.next();
    }    

    public int seasonalEffort(){
        return _treeState.seasonalEffort();
    }

    public String biologicalCycle(){
        return _treeState.biologicalCycle();
    }


    // Getter methods

    public String getTreeId(){
        return _treeId;
    }

    public String getTreeName(){
        return _treeName;
    }
    
    public int getAgeInYears(){
        return _treeAge / 4;
    }

    public int getTreeBaseCleaningDifficulty(){
        return _treeBaseCleaningDifficulty;
    }

    public SeasonState getTreeState(){
        return _treeState;
    }
    public abstract String getTreeType();

    // Setter methods

    public void setState(SeasonState state){
        _treeState = state;
    }
    public void incrementTreeAge(){
        _treeAge += 1;
    }

    public String toString(){
        return "ÁRVORE|" + _treeId + "|" + _treeName + "|"+ _treeAge / 4 + "|" + 
         _treeBaseCleaningDifficulty + "|" + getTreeType() + "|" 
         + biologicalCycle();
    }
}