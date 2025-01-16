package hva;

import java.io.Serializable;
import hva.Season;
import hva.Tree;

public class DeciduousTree extends Tree implements Serializable{
  
  /**
   * Constructor
   * @param treeId the tree's id
   * @param treeName the tree's name
   * @param treeAge the tree's age
   * @param treeBaseCleaningDifficulty the tree's base cleaning
   *                                   difficulty
   */
  public DeciduousTree(String treeId, String treeName, int treeAge,
    int treeBaseCleaningDifficulty){
      super(treeId, treeName, treeAge, treeBaseCleaningDifficulty);
  }

  public String getTreeType(){
      return "CADUCA";
  }

}