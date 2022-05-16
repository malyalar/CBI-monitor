## change filepath below to folder location of datatable_purple.csv
df <- read.csv("validation/datatable_purple.csv", header=TRUE)

library(ggplot2)
theme_set(
  theme_classic() +
    theme(legend.position = "top")
)

library(dplyr)
library(patchwork) # To display 2 charts together
library(hrbrthemes)

theme_pub <- function (base_size = 12, base_family = "") {
  
  theme_grey(base_size = base_size, 
             base_family = base_family) %+replace% 
    
    theme(# Set text size
      plot.title = element_text(size = 18),
      axis.title.x = element_text(size = 16),
      axis.title.y = element_text(size = 16, 
                                  angle = 90),
      
      axis.text.x = element_text(size = 14),
      axis.text.y = element_text(size = 14),
      
      strip.text.x = element_text(size = 15),
      strip.text.y = element_text(size = 15,
                                  angle = -90),
      
      # Legend text
      legend.title = element_text(size = 15),
      legend.text = element_text(size = 15),
      
      # Configure lines and axes
      axis.ticks.x = element_line(colour = "black"), 
      axis.ticks.y = element_line(colour = "black"), 
      
      # Plot background
      panel.background = element_rect(fill = "white"),
      panel.grid.major = element_line(colour = "grey83", 
                                      size = 0.2), 
      panel.grid.minor = element_line(colour = "grey88", 
                                      size = 0.5), 
      
      # Facet labels        
      legend.key = element_rect(colour = "grey80"), 
      strip.background = element_rect(fill = "grey80", 
                                      colour = "grey50", 
                                      size = 0.2))
}


#Unfortunately, ggplot will still, by default, label different conditions by color, rather than with different shades of grey. To fix this, use one of
#scale_fill_grey(start = 0, end = .9)
#scale_color_grey(start = 0, end = .9)



# calculate summary statistics for graphing
library(dplyr)
df <- subset(df, dilution_vv!=0)

df1.summary <- df %>%
  group_by(dilution_vv) %>%
  summarise(
    sd = sd(1/rp_value, na.rm = TRUE),
    mean = mean(1/rp_value)
  )

df1.summary
df1.summary <- subset(df1.summary, dilution_vv!=0)



df2.summary <- df %>%
  group_by(dilution_vv) %>%
  summarise(
    sd = sd(dilution_mmol, na.rm = TRUE),
    mean = mean(dilution_mmol)
  )

df2.summary
df2.summary <- subset(df2.summary, dilution_mmol!=0)


# start creating graph
p1 <- ggplot(
  df1.summary, 
  aes(x = dilution_vv, y = mean, ymin = mean-sd, ymax = mean+sd)
)

p1 + geom_errorbar(width = 0.1) +
  geom_point(size = 2, shape=1) +
  scale_x_log10(breaks=c(0, 0.02,0.04,0.07,0.14,0.25,0.5,1)) +
  scale_y_log10() +
  geom_line(aes(group = 1)) +
  labs(title = "Readouts on serial dilutions of blood", y= expression(1/log[10]~(purple)), x = expression(log[10]~(v/v~dilution))) +
  #labs(title = "Readouts on serial dilutions of blood", y= "purple", x = expression(log[10]~(v/v~dilution))) +
  theme_pub() +
  theme(plot.title = element_text(vjust=2), panel.grid.minor = element_blank())


ggsave("/Users/malyalar/Dropbox/Renal Transplant Research/hematuria monitor/figures/figures/raw_readouts_small.png",dpi=300, dev='png', height=5, width=5, units="in")
