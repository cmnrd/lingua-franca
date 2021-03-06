/* Instance of an action. */

/*************
Copyright (c) 2019, The University of California at Berkeley.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************/

package org.icyphy.graph

import java.util.LinkedHashMap
import java.util.LinkedHashSet
import java.util.List

/** 
 * Directed graph that maps nodes to its upstream and downstream neighbors. 
 * @author{Marten Lohstroh <marten@berkeley.edu>}
 */
class DirectedGraph<T> implements Graph<T> {
    
    /**
     * Adjacency map from vertices to their downstream neighbors.
     */
    var LinkedHashMap<T, LinkedHashSet<T>> originToEffects = new LinkedHashMap();
    
    /**
     * Adjacency map from vertices to their upstream neighbors.
     */
    var LinkedHashMap<T, LinkedHashSet<T>> effectToOrigins = new LinkedHashMap();
    
    
    /**
     * Construct a new dependency graph.
     */
    new() {
        
    }
    
    protected def graphChanged() {
        // To be overridden by subclasses that perform analysis.
    }
    
    /**
     * Return as to whether this graph as the given node in it.
     * @param node The node to look for.
     */
    override hasNode(T node) {
        if (this.effectToOrigins.get(node) !== null || originToEffects.get(node) !== null) {
            true
        } else {
           false 
        }
    }
    
    /**
     * Return all upstream neighbors of a given node.
     * @param node The node to report the upstream neighbors of.
     */
    def getOrigins(T node) {
        var origins = this.effectToOrigins.get(node)
        if (origins === null) {
            return emptyList
        } else {
            return origins.toList
        }
    }

    /**
     * Return all upstream neighbors of a given node.
     * @param node The node to report the downstream neighbors of.
     */
    def getEffects(T node) {
        var effects = this.originToEffects.get(node)
        if (effects === null) {
            return emptyList
        } else {
            return effects.toList
        }
    }

    
    /**
     * Add the given node to the graph.
     * @param node The node to add to the graph.
     */
    override void addNode(T node) {
        this.graphChanged()
        if (!effectToOrigins.containsKey(node)) {
            this.effectToOrigins.put(node, new LinkedHashSet<T>())    
        }
        if (!originToEffects.containsKey(node)) {
            this.originToEffects.put(node, new LinkedHashSet<T>())
        }
    }
    
    /**
     * Remove the given node from the graph. This also eliminates any
     * edges from upstream and to downstream neighbors of this node.
     * @param The node to remove.
     */
    override removeNode(T node) {
        this.graphChanged()
        this.effectToOrigins.remove(node)
        this.effectToOrigins.forEach[v, e | e.remove(node)]
        
        this.originToEffects.remove(node)
        this.originToEffects.forEach[v, e | e.remove(node)]
    }
    
    /**
     * Add a new directed edge to the graph. The first argument is
     * the downstream node, the second argument the upstream node.
     * @param effect The downstream neighbor.
     * @param origin The upstream neighbor.
     */
    override addEdge(T effect, T origin) {
        this.graphChanged()
        var effects = this.originToEffects.get(origin)
        var origins = this.effectToOrigins.get(effect)
        if (effects === null) {
            effects = newLinkedHashSet
            this.originToEffects.put(origin, effects)
        }
        if (origins === null) {
            origins =  newLinkedHashSet
            this.effectToOrigins.put(effect, origins)
        }
        effects.add(effect)
        origins.add(origin)
    }
    
    /**
     * Add new directed edges to the graph. The first argument is the
     * downstream node, the second argument a set of upstream nodes.
     * @param effect The downstream neighbor.
     * @param origins The upstream neighbors.
     */
    override addEdges(T effect, List<T> origins) {
        this.graphChanged()
        for (origin : origins) {
            this.addEdge(effect, origin)
        }
    }
    
    /**
     * Remove a directed edge from the graph.
     * @param effect The downstream neighbor.
     * @param origin The upstream neighbor.
     */
    override removeEdge(T effect, T origin) {
        this.graphChanged()
        var origins = this.effectToOrigins.get(effect)
        var effects = this.originToEffects.get(origin)
        if (origins !== null && origins.contains(origin)) {
            origins.remove(origin)
        }
        if (effects !== null && effects.contains(effect)) {
            effects.remove(effect)
        }
    }
    
    /**
     * Obtain a copy of this graph by creating an new instance and copying
     * the adjacency maps.
     */
    def copy() {
        val graph = new DirectedGraph<T>()
        for (entry : this.effectToOrigins.entrySet) {
            graph.effectToOrigins.put(entry.key, new LinkedHashSet(entry.value))    
        }
        for (entry : this.originToEffects.entrySet) {
            graph.originToEffects.put(entry.key, new LinkedHashSet(entry.value))    
        }
        return graph
    }
    
    /**
     * For a given a two adjacency maps, copy missing edges from the first
     * map to the second.
     * @param srcMap The adjacency map to copy edges from.
     * @param dstMap The adjacency map to copy edges to.
     */
    private def void mirror(LinkedHashMap<T, LinkedHashSet<T>> srcMap, LinkedHashMap<T, LinkedHashSet<T>> dstMap) {
        if (srcMap !== null && dstMap !== null) {
            for (node : srcMap.keySet) {
                val srcEdges = srcMap.get(node)
                val dstEdges = dstMap.get(node)
                if (dstEdges === null) {
                    // Node does not exist; add it.
                    dstMap.put(node, srcEdges)
                } else {
                    // Node does exist; add the missing edges.
                    for (edge : srcEdges) {
                        if (!dstEdges.contains(edge)) {
                            dstEdges.add(edge)
                        }
                    }
                } 
            }    
        }
    }
    
    /**
     * Merge another directed graph into this one.
     * @param another The graph to merge into this one.
     */
    def merge(DirectedGraph<T> another) {
        this.graphChanged()
        mirror(another.effectToOrigins, this.effectToOrigins)
        mirror(another.originToEffects, this.originToEffects)
    }
    
    /**
     * Return the set of nodes that have no neighbors listed in the given
     * adjacency map.
     */
    private def independentNodes(LinkedHashMap<T, LinkedHashSet<T>> adjacencyMap) {
        var independent = new LinkedHashSet<T>()
        for (node : this.nodes) {
            val neighbors = adjacencyMap.get(node)
            if (neighbors === null || neighbors.size == 0) {
                independent.add(node)
            }
        }
        return independent.toList
    }
    
    /**
     * Return the root nodes of this graph.
     * Root nodes have no upstream neighbors.
     */
    def rootNodes() {
        return independentNodes(this.effectToOrigins)
    }
    
    /**
     * Return the leaf nodes of this graph.
     * Leaf nodes have no downstream neighbors.
     */
    def leafNodes() {
        return independentNodes(this.originToEffects)
    }
    
    /**
     * Report the number of nodes in this graph.
     */
    override nodeCount() {
        this.nodes.size
    }

    /**
     * Report the number of directed edges in this graph.
     */    
    override edgeCount() {
       var edges = 0
       for(effect : this.effectToOrigins.keySet) {
           edges += this.effectToOrigins.get(effect).size
       }
       for(origin: this.originToEffects.keySet) {
           for (effect : this.originToEffects.get(origin)) {
               if (this.effectToOrigins.get(effect) === null) {
                   edges++ // Account for possible asymmetry.
               }
           }
       }
       edges
    }
    
    /**
     * Return the nodes in this graph.
     */
    override nodes() {
        val nodes = newLinkedHashSet
        nodes.addAll(this.effectToOrigins.keySet)
        nodes.addAll(this.originToEffects.keySet)
        return nodes.toList
    }
    
}